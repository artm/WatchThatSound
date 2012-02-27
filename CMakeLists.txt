PROJECT(WatchThatSound)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_MODULE_PATH
  "${WatchThatSound_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
INCLUDE(WTS-Package)
INCLUDE(ConfigUtils)
CONFIGURE_FILE(wts_version.h.in wts_version.h)
SET(CMAKE_INCLUDE_CURRENT_DIR true)
# this helps QtCreator's completion use generated files
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(CMAKE_AUTOMOC 1)
FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtNetwork Phonon)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(FFmpeg REQUIRED)
INCLUDE_DIRECTORIES(${FFmpeg_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FFmpeg_CXX_FLAGS}")
# Disable warnings caused by the ffmpeg headers
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -Wno-attributes")

IF(APPLE)
  FIND_LIBRARY_VERBOSE(Sparkle Sparkle)
  FIND_LIBRARY_VERBOSE(AppKit AppKit)
  SET(Sparkle_LIBRARIES ${Sparkle_LIBRARY} ${AppKit_LIBRARY})
ENDIF(APPLE)

FIND_PACKAGE(Portaudio REQUIRED)
INCLUDE_DIRECTORIES(${Portaudio_INCLUDE_DIRS})

FIND_LIBRARY_VERBOSE(SndFile sndfile)

INCLUDE(SrcScan)
SCAN_GLOB(WatchThatSound_SOURCES "*.cpp" "C++ sources")
SCAN_GLOB(WatchThatSound_UI "*.ui" "Qt Forms (*.ui)")
SCAN_GLOB(WatchThatSound_RESOURCES "*.qrc" "Qt Resources (*.qrc)")
IF(APPLE)
  SCAN_GLOB(WatchThatSound_OBJC_SOURCES "*.mm" "Objective-C sources (*.mm)")
ENDIF(APPLE)
SCAN_DONE()

QT4_WRAP_UI(WatchThatSound_HEADERS_UI ${WatchThatSound_UI})
QT4_ADD_RESOURCES(WatchThatSound_RCC ${WatchThatSound_RESOURCES})

SET(MACOSX_BUNDLE_GUI_IDENTIFIER "nl.v2.${PROJECT_NAME}")
SET(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})

SET(WatchThatSound_CXX_SOURCES
  ${WatchThatSound_SOURCES}
  ${WatchThatSound_HEADERS_MOC}
  ${WatchThatSound_HEADERS_UI}
  ${WatchThatSound_RCC})

# Precompiled headers support
INCLUDE("Gch")
ADD_PCH_RULE(stable.h WatchThatSound_CXX_SOURCES)
# This speeds up `make clean all -j 5` by 30 seconds on core i3 CPU
SET_PROPERTY(DIRECTORY PROPERTY CLEAN_NO_CUSTOM true)

IF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  SET(GUI WIN32)
ENDIF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")

ADD_EXECUTABLE(${APPLICATION_NAME}
               ${GUI}
               MACOSX_BUNDLE
               ${WatchThatSound_CXX_SOURCES}
               ${WatchThatSound_OBJC_SOURCES})

SET(WatchThatSound_LIBRARIES
  ${QT_LIBRARIES}
  ${FFmpeg_LIBRARIES}
  ${Portaudio_LIBRARIES}
  ${Sparkle_LIBRARIES}
  ${SndFile_LIBRARY})

TARGET_LINK_LIBRARIES(${APPLICATION_NAME} ${WatchThatSound_LIBRARIES})

SET_TARGET_PROPERTIES(${APPLICATION_NAME}
  PROPERTIES
  LINK_FLAGS "${PKG_Portaudio_LDFLAGS_OTHER}"
  MACOSX_BUNDLE true
  MACOSX_BUNDLE_BUNDLE_NAME "${PACKAGE_NAME}"
  MACOSX_BUNDLE_INFO_STRING "${PACKAGE_NAME}"
  MACOSX_BUNDLE_BUNDLE_VERSION "${PACKAGE_VERSION}"
  MACOSX_BUNDLE_LONG_VERSION_STRING "${PACKAGE_VERSION}"
  MACOSX_BUNDLE_SHORT_VERSION_STRING "${PACKAGE_VERSION}"
  MACOSX_BUNDLE_ICON_FILE "${CMAKE_SOURCE_DIR}/WTS.icns"
  MACOSX_BUNDLE_GUI_IDENTIFIER "NL.WatchThatSound"
  MACOSX_BUNDLE_COPYRIGHT "Stichting Watch That Sound / V2_")

IF(APPLE)
  ADD_CUSTOM_COMMAND(
    TARGET ${APPLICATION_NAME} POST_BUILD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND rm -rf ${APPLICATION_NAME}.app/Contents/{Frameworks,PlugIns,Resources/qt.conf}
    COMMAND ${QT_BINARY_DIR}/macdeployqt ${APPLICATION_NAME}.app > ${PROJECT_NAME}-macdeployqt.log 2>&1
    # macdeploy misses Sparkle
    COMMAND cp -R ${Sparkle_LIBRARY} ${APPLICATION_NAME}.app/Contents/Frameworks/
    COMMENT "Fixing up the app bundle")
ENDIF(APPLE)

INSTALL(
  TARGETS ${APPLICATION_NAME}
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION .)

IF(WIN32)
  FILE(GLOB VIDEO_SAMPLES dist-movie/*.avi)
  MESSAGE(STATUS "Distribution videos: ${VIDEO_SAMPLES}")

  INSTALL(
    FILES ${VIDEO_SAMPLES}
    DESTINATION video)
ENDIF(WIN32)

IF(WIN32)
  INCLUDE(TakeAway)
  COLLECT_TAKEAWAY(WatchThatSound_LIBRARIES)
  SET(WatchThatSound_QT_PLUGINS
    "phonon_backend;codecs;iconengines;imageformats;graphicssystems")
  COLLECT_TAKEAWAY_QT_PLUGINS(WatchThatSound_QT_PLUGINS)
ENDIF(WIN32)
