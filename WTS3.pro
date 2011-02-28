# -------------------------------------------------
# Project created by QtCreator 2011-02-15T13:57:43
# -------------------------------------------------
QT += opengl \
    script \
    svg \
    webkit \
    phonon \
    multimedia
TARGET = WTS3
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    timelinewidget.cpp \
    soundbuffer.cpp \
    sequencertimeline.cpp \
    rainbow.cpp \
    waveformwidget.cpp \
    wtsaudio.cpp
HEADERS += mainwindow.h \
    timelinewidget.h \
    soundbuffer.h \
    sequencertimeline.h \
    rainbow.h \
    waveformwidget.h \
    wtsaudio.h
FORMS += mainwindow.ui
RESOURCES += WTS3Resources.qrc

INCLUDEPATH += PortAudio/portaudio/include
LIBS += -LPortAudio/portaudio/lib -lportaudio
macx:LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices

OTHER_FILES += \
    PLAN.txt
