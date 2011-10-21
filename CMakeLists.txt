PROJECT(WatchThatSound)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(PROJECT_VERSION 3.0.3)
CONFIGURE_FILE(wts_version.h.in wts_version.h)
SET(CMAKE_INCLUDE_CURRENT_DIR true)

MESSAGE(STATUS "
Welcome to WatchThatSound build system.

Example usage (windows):
  C:/Program\\ Files\\ \\(x86\\)/CMake\\ 2.8/bin/cmake.exe \\
    ../WatchThatSound \\
    -G 'MSYS Makefiles' \\
    -DQT_QMAKE_EXECUTABLE=C:/QtSDK/Desktop/Qt/4.8.0/mingw/bin/qmake.exe \\
    -DPortaudio_SRCDIR=../portaudio \\
    -DFFmpeg_SRCDIR=../ffmpeg \\
    -DCMAKE_BUILD_TYPE=Debug
")

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

ADD_EXECUTABLE(WatchThatSound
               MACOSX_BUNDLE
               ${WatchThatSound_ALL_SOURCES})

TARGET_LINK_LIBRARIES(WatchThatSound
  ${QT_LIBRARIES}
  ${FFmpeg_LIBRARIES}
  ${Portaudio_LIBRARIES})
