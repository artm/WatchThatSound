PROJECT(WatchThatSound)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(PROJECT_NAME_SHORT "WatchThatSoundTool")
SET(PROJECT_NAME_LONG "Watch That Sound Tool")
SET(PROJECT_EXE_NAME ${PROJECT_NAME_SHORT})

SET(CPACK_PACKAGE_NAME ${PROJECT_NAME_SHORT})
SET(CPACK_PACKAGE_VENDOR "Stichting Wathc That Sound / V2_ Lab")
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "5")
SET(PROJECT_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "A soundtrack composition worksop tool developed by V2_ Lab for Watch
  That Sound foundation")
SET(CPACK_PACKAGE_FILE_NAME "Setup-${PROJECT_NAME}-${PROJECT_VERSION}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME_LONG})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/Readme.md")
SET(CPACK_MONOLITHIC_INSTALL true)
SET(CPACK_PACKAGE_EXECUTABLES "${PROJECT_EXE_NAME};${PROJECT_NAME_LONG}")
SET(CPACK_STRIP_FILES true)
INCLUDE(CPack)

CONFIGURE_FILE(wts_version.h.in wts_version.h)
SET(CMAKE_INCLUDE_CURRENT_DIR true)

SET(CMAKE_MODULE_PATH "${WatchThatSound_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

SET(QT_USE_PHONON true)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(FFmpeg REQUIRED)
INCLUDE_DIRECTORIES(${FFmpeg_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FFmpeg_CXX_FLAGS}")
# Disable warnings caused by the ffmpeg headers
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -Wno-attributes")

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
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winvalid-pch -I${CMAKE_CURRENT_BINARY_DIR} -include stable.h")
# This speeds up `make clean all -j 5` by 30 seconds on core i3 CPU
SET_PROPERTY(DIRECTORY PROPERTY CLEAN_NO_CUSTOM true)

IF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  SET(GUI WIN32)
ENDIF(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")

ADD_EXECUTABLE(${PROJECT_EXE_NAME}
               ${GUI}
               MACOSX_BUNDLE
               ${WatchThatSound_ALL_SOURCES})

SET(WatchThatSound_LIBRARIES
  ${QT_LIBRARIES}
  ${FFmpeg_LIBRARIES}
  ${Portaudio_LIBRARIES})

TARGET_LINK_LIBRARIES(${PROJECT_EXE_NAME}
  ${WatchThatSound_LIBRARIES})

INSTALL(
  TARGETS ${PROJECT_EXE_NAME}
  RUNTIME DESTINATION bin)


INCLUDE(TakeAway)
COLLECT_TAKEAWAY(WatchThatSound_LIBRARIES)
SET(WatchThatSound_QT_PLUGINS
  "phonon_backend;codecs;iconengines;imageformats;graphicssystems")
COLLECT_TAKEAWAY_QT_PLUGINS(WatchThatSound_QT_PLUGINS)

