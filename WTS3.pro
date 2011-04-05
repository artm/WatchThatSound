QT += phonon multimedia
TARGET = WatchThatSound

VERSION = 3-alpha-10.1

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

  QMAKE_INFO_PLIST=WTS.plist

  QMAKE_POST_LINK = mkdir -p "$$TARGET".app/Contents/Frameworks \
    && cp -r /Library/Frameworks/Sparkle.framework "$$TARGET".app/Contents/Frameworks \
    && cat "$$TARGET".app/Contents/Info.plist | sed s/@VERSION@/$$VERSION/g > "$$TARGET".app/Contents/Info.plist.new \
    && mv "$$TARGET".app/Contents/Info.plist.new "$$TARGET".app/Contents/Info.plist
}
