QT += phonon multimedia
TARGET = WTS3
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
    scoresymbol.cpp
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
    scoresymbol.h
FORMS += mainwindow.ui
RESOURCES += WTS3Resources.qrc
CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

ICON = WTS.icns

INCLUDEPATH += Shoulders/portaudio/include Shoulders/ffmpeg
LIBS += -L$$PWD/Shoulders/portaudio/lib -lportaudio
LIBS += -L$$PWD/Shoulders/ffmpeg/libavcodec -lavcodec
LIBS += -L$$PWD/Shoulders/ffmpeg/libavformat -lavformat -lz -lbz2
LIBS += -L$$PWD/Shoulders/ffmpeg/libavutil -lavutil
LIBS += -L$$PWD/Shoulders/ffmpeg/libswscale -lswscale

macx:LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices

OTHER_FILES += \
    MEMO.txt \
    DEVLOG.txt
