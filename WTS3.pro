VERSION = 3-alpha-11.0
TARGET = WatchThatSound

QT += phonon multimedia

DEFINES += WTS_VERSION=\\\"$$VERSION\\\"

TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    timelinewidget.cpp \
    soundbuffer.cpp \
    sequencertimeline.cpp \
    rainbow.cpp \
    waveformwidget.cpp \
    wtsaudio.cpp \
    storyboard.cpp \
    videofile.cpp \
    synced.cpp \
    exporter.cpp \
    curveeditor.cpp \
    scoreeditor.cpp \
    scoresymbol.cpp \
    AutoUpdater.cpp
HEADERS += mainwindow.h \
    timelinewidget.h \
    soundbuffer.h \
    sequencertimeline.h \
    rainbow.h \
    waveformwidget.h \
    wtsaudio.h \
    storyboard.h \
    videofile.h \
    synced.h \
    exporter.h \
    stable.h \
    curveeditor.h \
    scoreeditor.h \
    scoresymbol.h \
    AutoUpdater.h
FORMS += mainwindow.ui
RESOURCES += WTS3Resources.qrc
CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

INCLUDEPATH += Shoulders/portaudio/include Shoulders/ffmpeg
LIBS += -L$$PWD/Shoulders/portaudio/lib -lportaudio
LIBS += -L$$PWD/Shoulders/ffmpeg/libavcodec -lavcodec
LIBS += -L$$PWD/Shoulders/ffmpeg/libavformat -lavformat -lz -lbz2
LIBS += -L$$PWD/Shoulders/ffmpeg/libavutil -lavutil
LIBS += -L$$PWD/Shoulders/ffmpeg/libswscale -lswscale

OTHER_FILES += \
    MEMO.txt \
    DEVLOG.txt \
    appcast.xml \
    WTS.plist \
    .gitignore

mac {
  ICON = WTS.icns

  HEADERS += \
    SparkleAutoUpdater.h \
    CocoaInitializer.h

  OBJECTIVE_SOURCES += \
    SparkleAutoUpdater.mm \
    CocoaInitializer.mm

  LIBS += -framework Sparkle -framework AppKit
  LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices

  InfoPlist.target = "$$TARGET".app/Contents/Info.plist
  InfoPlist.source = WTS.plist
  InfoPlist.depends = $$InfoPlist.source WTS3.pro
  InfoPlist.commands = sed \"s/@VERSION@/$$VERSION/g;s/@EXECUTABLE@/$$TARGET/g;s/@TYPEINFO@/WTS3/g;s/@ICON@/$$ICON/g\" \
                       $$InfoPlist.source > $$InfoPlist.target

  QMAKE_EXTRA_TARGETS += InfoPlist

  QMAKE_POST_LINK = mkdir -p "$$TARGET".app/Contents/Frameworks \
    && cp -r /Library/Frameworks/Sparkle.framework "$$TARGET".app/Contents/Frameworks
}
