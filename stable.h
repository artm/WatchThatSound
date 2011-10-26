#ifndef STABLE_H
#define STABLE_H

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include <portaudio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#if defined __cplusplus
#include <QtCore>
#include <QtGui>
#include <Phonon>
#endif

#endif // STABLE_H
