PROJECT(WatchThatSound)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(PROJECT_NAME_LONG "Watch That Sound Tool")
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "3")
SET(PROJECT_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
IF(WIN32)
  SET(CPACK_GENERATOR NSIS)
  # on windows we build an executable with a short name for the sake of
  # mingw. But the start menu item and eventual desktop pictogram use
  # the long name so it's not a problem.
  SET(APPLICATION_NAME ${PROJECT_NAME})
  SET(CPACK_PACKAGE_FILE_NAME "Setup-${PROJECT_NAME}-${PROJECT_VERSION}")
ELSE(WIN32)
  SET(CPACK_GENERATOR DragNDrop)
  SET(APPLICATION_NAME "${PROJECT_NAME_LONG}")
  SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
ENDIF(WIN32)

SET(CPACK_PACKAGE_VENDOR "Stichting Watch That Sound / V2_ Lab")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "A soundtrack composition worksop tool developed by V2_ Lab for Watch That Sound foundation")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME_LONG})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
# TODO convert markdown to html
CONFIGURE_FILE(README.md README.txt)
SET(CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/Readme.txt")
SET(CPACK_MONOLITHIC_INSTALL true)
SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME};${PROJECT_NAME_LONG}")
SET(CPACK_STRIP_FILES true)
SET(CPACK_DMG_VOLUME_NAME ${APPLICATION_NAME}-${PROJECT_VERSION})
SET(CPACK_DMG_FORMAT UDBZ)
INCLUDE(CPack)

CONFIGURE_FILE(wts_version.h.in wts_version.h)
SET(CMAKE_INCLUDE_CURRENT_DIR true)

SET(CMAKE_MODULE_PATH "${WatchThatSound_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui Phonon)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(FFmpeg REQUIRED)
INCLUDE_DIRECTORIES(${FFmpeg_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FFmpeg_CXX_FLAGS}")
# Disable warnings caused by the ffmpeg headers
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -Wno-attributes")

MACRO(FIND_LIBRARY_VERBOSE VAR Lib)
  IF(NOT ${VAR}_LIBRARY)
    FIND_LIBRARY(${VAR}_LIBRARY ${Lib})
    IF("${${VAR}_LIBRARY}" STREQUAL "")
      MESSAGE(SEND_ERROR "Required library ${VAR} not found")
    ELSE("${${VAR}_LIBRARY}" STREQUAL "")
      MESSAGE(STATUS "Found ${VAR}: ${${VAR}_LIBRARY}")
    ENDIF("${${VAR}_LIBRARY}" STREQUAL "")
  ENDIF(NOT ${VAR}_LIBRARY)
ENDMACRO(FIND_LIBRARY_VERBOSE VAR Lib)

IF(APPLE)
  FIND_LIBRARY_VERBOSE(Sparkle Sparkle)
  FIND_LIBRARY_VERBOSE(AppKit AppKit)
  SET(Sparkle_LIBRARIES ${Sparkle_LIBRARY} ${AppKit_LIBRARY})

  SET(WatchThatSound_OBJC_SOURCES
    SparkleAutoUpdater.mm
    CocoaInitializer.mm)
ENDIF(APPLE)

FIND_PACKAGE(Portaudio REQUIRED)
INCLUDE_DIRECTORIES(${Portaudio_INCLUDE_DIRS})

SET(WatchThatSound_SOURCES
    main.cpp
    MainWindow.cpp
    SoundBuffer.cpp
    TimeLineWidget.cpp
    SequencerTimeLine.cpp
    Rainbow.cpp
    WaveformWidget.cpp
    WtsAudio.cpp
    StoryBoard.cpp
    VideoFile.cpp
    Synced.cpp
    Exporter.cpp
    CurveEditor.cpp
    ScoreEditor.cpp
    ScoreSymbol.cpp
    AutoUpdater.cpp
    VuMeter.cpp
    BufferItem.cpp
    SharpLine.cpp
    TimeLineItem.cpp
    Preferences.cpp)

SET(WatchThatSound_QT_HEADERS
    MainWindow.h
    TimeLineWidget.h
    SequencerTimeLine.h
    WaveformWidget.h
    WtsAudio.h
    StoryBoard.h
    VideoFile.h
    Synced.h
    Exporter.h
    CurveEditor.h
    ScoreEditor.h
    VuMeter.h
    Preferences.h)

LIST(APPEND WatchThatSound_HEADERS
            Common.h
            AutoUpdater.h
            Rainbow.h
            SoundBuffer.h
            ScoreSymbol.h
            BufferItem.h
            SharpLine.h
            TimeLineItem.h)

SET(WatchThatSound_UI
    mainwindow.ui
    Preferences.ui)

SET(WatchThatSound_RESOURCES WTS3Resources.qrc)

QT4_WRAP_CPP(WatchThatSound_HEADERS_MOC ${WatchThatSound_QT_HEADERS})
QT4_WRAP_UI(WatchThatSound_HEADERS_UI ${WatchThatSound_UI})
QT4_ADD_RESOURCES(WatchThatSound_RCC ${WatchThatSound_RESOURCES})

SET(MACOSX_BUNDLE_GUI_IDENTIFIER "nl.v2.${PROJECT_NAME}")
SET(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})

SET(WatchThatSound_ALL_SOURCES
  ${WatchThatSound_SOURCES}
  ${WatchThatSound_HEADERS_MOC}
  ${WatchThatSound_HEADERS_UI}
  ${WatchThatSound_RCC})

# Precompiled headers support
INCLUDE("Gch")
ADD_PCH_RULE(stable.h WatchThatSound_ALL_SOURCES)
SET(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Winvalid-pch -I${CMAKE_CURRENT_BINARY_DIR} -include stable.h")
# This speeds up `make clean all -j 5` by 30 seconds on core i3 CPU
SET_PROPERTY(DIRECTORY PROPERTY CLEAN_NO_CUSTOM true)

IF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  SET(GUI WIN32)
ENDIF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")

ADD_EXECUTABLE(${APPLICATION_NAME}
               ${GUI}
               MACOSX_BUNDLE
               ${WatchThatSound_ALL_SOURCES}
               ${WatchThatSound_OBJC_SOURCES})

SET(WatchThatSound_LIBRARIES
  ${QT_LIBRARIES}
  ${FFmpeg_LIBRARIES}
  ${Portaudio_LIBRARIES}
  ${Sparkle_LIBRARIES})

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
    # macdeploy misses Sparkle
    COMMAND rm -rf ${APPLICATION_NAME}.app/Contents/{Frameworks,PlugIns,Resources/qt.conf}
    COMMAND ${QT_BINARY_DIR}/macdeployqt ${APPLICATION_NAME}.app > ${PROJECT_NAME}-macdeployqt.log 2>&1
    COMMAND cp -R ${Sparkle_LIBRARY} ${APPLICATION_NAME}.app/Contents/Frameworks/
    COMMENT "Fixing up the app bundle")
ENDIF(APPLE)

INSTALL(
  TARGETS ${APPLICATION_NAME}
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION .)

IF(WIN32)
  INCLUDE(TakeAway)
  COLLECT_TAKEAWAY(WatchThatSound_LIBRARIES)
ENDIF(WIN32)
