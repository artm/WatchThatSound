VERSION = 3.0.2
mac {
  TARGET = "Watch That Sound tool"
}
win32 {
  TARGET = "WatchThatSoundTool"
}

QT += phonon multimedia
CONFIG += precompile_header

WtsVersion.target = $$OUT_PWD/wts_version.h
WtsVersion.source = $$PWD/wts_version.h.in
WtsVersion.depends = $$WtsVersion.source $$PWD/WTS3.pro
WtsVersion.commands = sed \"s/@VERSION@/$$VERSION/g\" $$WtsVersion.source > "$$WtsVersion.target".tmp \
  && diff "$$WtsVersion.target".tmp "$$WtsVersion.target" > /dev/null \
  || mv "$$WtsVersion.target".tmp "$$WtsVersion.target"

QMAKE_EXTRA_TARGETS += WtsVersion
PRE_TARGETDEPS += $$OUT_PWD/wts_version.h

TEMPLATE = app
SOURCES += main.cpp \
    MainWindow.cpp \
    SoundBuffer.cpp \
    TimeLineWidget.cpp \
    SequencerTimeLine.cpp \
    Rainbow.cpp \
    WaveformWidget.cpp \
    WtsAudio.cpp \
    StoryBoard.cpp \
    VideoFile.cpp \
    Synced.cpp \
    Exporter.cpp \
    CurveEditor.cpp \
    ScoreEditor.cpp \
    ScoreSymbol.cpp \
    AutoUpdater.cpp \
    VuMeter.cpp \
    BufferItem.cpp \
    SharpLine.cpp \
    TimeLineItem.cpp \
    Preferences.cpp
HEADERS += MainWindow.h \
    TimeLineWidget.h \
    SoundBuffer.h \
    SequencerTimeLine.h \
    Rainbow.h \
    WaveformWidget.h \
    WtsAudio.h \
    StoryBoard.h \
    VideoFile.h \
    Synced.h \
    Exporter.h \
    stable.h \
    CurveEditor.h \
    ScoreEditor.h \
    ScoreSymbol.h \
    AutoUpdater.h \
    VuMeter.h \
    BufferItem.h \
    SharpLine.h \
    TimeLineItem.h \
    Preferences.h
FORMS += mainwindow.ui \
    Preferences.ui
RESOURCES += WTS3Resources.qrc
PRECOMPILED_HEADER = stable.h
OTHER_FILES += \
    MEMO.txt \
    DEVLOG.txt \
    appcast.xml \
    WTS.plist \
    .gitignore \
    wts_version.h.in \
    LICENSE.txt


INCLUDEPATH += $$PWD/Shoulders/portaudio/include $$PWD/Shoulders/ffmpeg

win32 {
  LIBS += -L$$PWD/Shoulders/portaudio/lib/.libs -lportaudio -lwinmm -lm -lole32 -luuid
  LIBS += -L$$PWD/Shoulders/ffmpeg/libavformat -lavformat -lavicap32 -lm -lpsapi
  LIBS += -L$$PWD/Shoulders/ffmpeg/libavcodec -lavcodec
  LIBS += -L$$PWD/Shoulders/ffmpeg/libavutil -lavutil
  LIBS += -L$$PWD/Shoulders/ffmpeg/libswscale -lswscale
}

mac {
  ICON = WTS.icns

  HEADERS += \
    SparkleAutoUpdater.h \
    CocoaInitializer.h

  OBJECTIVE_SOURCES += \
    SparkleAutoUpdater.mm \
    CocoaInitializer.mm

  LIBS += $$PWD/Shoulders/portaudio/lib/.libs/libportaudio.a
  LIBS += $$PWD/Shoulders/ffmpeg/libavcodec/libavcodec.a
  LIBS += $$PWD/Shoulders/ffmpeg/libavformat/libavformat.a -lz -lbz2
  LIBS += $$PWD/Shoulders/ffmpeg/libavutil/libavutil.a
  LIBS += $$PWD/Shoulders/ffmpeg/libswscale/libswscale.a

  LIBS += -framework Sparkle -framework AppKit
  LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices

  InfoPlist.target = "$$TARGET".app/Contents/Info.plist
  InfoPlist.source = $$PWD/WTS.plist
  InfoPlist.depends = $$InfoPlist.source $$PWD/WTS3.pro
  InfoPlist.commands = sed \"s/@VERSION@/$$VERSION/g;s/@EXECUTABLE@/$$TARGET/g;s/@TYPEINFO@/WTS3/g;s/@ICON@/$$ICON/g\" \
                       \"$$InfoPlist.source\" > \"$$InfoPlist.target\"

  QMAKE_EXTRA_TARGETS += InfoPlist

  QMAKE_POST_LINK = mkdir -p \"$$TARGET\".app/Contents/Frameworks \
    && cp -r /Library/Frameworks/Sparkle.framework \"$$TARGET\".app/Contents/Frameworks
}
