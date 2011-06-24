VERSION = 3.0.1
TARGET = "Watch That Sound tool"

QT += phonon

WtsVersion.target = $$quote($$OUT_PWD/wts_version.h)
WtsVersion.source = $$quote($$PWD/wts_version.h.in)
WtsVersion.depends = $$quote($$WtsVersion.source) $$quote($$PWD/WTS3.pro)
WtsVersion.commands = sed \"s/@VERSION@/$$VERSION/g\" $$quote($$WtsVersion.source) > $$quote($$WtsVersion.target).tmp \
  && diff $$quote($$WtsVersion.target).tmp $$quote($$WtsVersion.target) > /dev/null \
  || mv $$quote($$WtsVersion.target).tmp $$quote($$WtsVersion.target)

QMAKE_EXTRA_TARGETS += WtsVersion
PRE_TARGETDEPS += $$quote($$OUT_PWD/wts_version.h)

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
CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

LIBS += -lportaudio
LIBS += -lavcodec -lavformat -lz -lbz2 -lavutil -lswscale

OTHER_FILES += \
    MEMO.txt \
    DEVLOG.txt \
    appcast.xml \
    WTS.plist \
    .gitignore \
    wts_version.h.in \
    LICENSE.txt

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

  InfoPlist.target = $$quote($$TARGET).app/Contents/Info.plist
  InfoPlist.source = $$quote($$PWD/WTS.plist)
  InfoPlist.depends = $$quote($$InfoPlist.source) $$quote($$PWD/WTS3.pro)
  InfoPlist.commands = sed \"s/@VERSION@/$$VERSION/g;s/@EXECUTABLE@/$$TARGET/g;s/@TYPEINFO@/WTS3/g;s/@ICON@/$$ICON/g\" \
                       \"$$quote($$InfoPlist.source)\" > \"$$quote($$InfoPlist.target)\"

  QMAKE_EXTRA_TARGETS += InfoPlist

  QMAKE_POST_LINK = mkdir -p \"$$quote($$TARGET).app/Contents/Frameworks\" \
    && cp -r /Library/Frameworks/Sparkle.framework \"$$quote($$TARGET).app/Contents/Frameworks\"
}










