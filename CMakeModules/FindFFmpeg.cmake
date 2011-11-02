# Try to find FFmpeg, paying special attention to its uninstalled builds.
#
# Defines:
#
#   FFmpeg_FOUND (true/false)
#
# if found, defines additionally:
#
#   FFmpeg_INCLUDE_DIRS
#   FFmpeg_LIBRARIES
#   FFmpeg_CXX_FLAGS
#
#  Copyright 2011 Watch that Sound / V2_ Lab
#
#  Based on scripts by Andreas Schneider and Lasse Kärkkäinen:
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org> Modified for other
#  libraries by Lasse Kärkkäinen <tronic>
#
#  Redistribution and use is allowed according to the terms of the New BSD
#  license.
#

SET(FFmpeg_BUILDDIR CACHE PATH "Path to FFmpeg build directory")
SET(FFmpeg_SRCDIR CACHE PATH "Path to FFmpeg source directory")

IF (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
  # in cache already
  SET(FFmpeg_FOUND TRUE)
ELSE (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
  INCLUDE(LibFindMacros)

  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  FIND_PACKAGE(PkgConfig)

  SET(COMPONENTS avformat avcodec avutil swscale)
  # can't seem to find this anywhere
  IF(WIN32)
    SET(dirsep ";")
  ELSE(WIN32)
    SET(dirsep ":")
  ENDIF(WIN32)

  IF(NOT FFmpeg_BUILDDIR AND FFmpeg_SRCDIR)
    # in source build
    SET(FFmpeg_BUILDDIR ${FFmpeg_SRCDIR})
  ENDIF(NOT FFmpeg_BUILDDIR AND FFmpeg_SRCDIR)

  IF(FFmpeg_BUILDDIR)
    FOREACH(COMPONENT ${COMPONENTS})
      SET(ENV{PKG_CONFIG_PATH}
          "${FFmpeg_BUILDDIR}/lib${COMPONENT}${dirsep}$ENV{PKG_CONFIG_PATH}")
    ENDFOREACH(COMPONENT ${COMPONENTS})
  ENDIF(FFmpeg_BUILDDIR)

  MACRO(FFmpeg_FIND COMPONENT)
    IF (PKG_CONFIG_FOUND)

      SET(MODULE lib${COMPONENT})
      IF(FFmpeg_BUILDDIR)
        SET(MODULE lib${COMPONENT}-uninstalled)
      ENDIF(FFmpeg_BUILDDIR)

      PKG_CHECK_MODULES(FFmpeg_${COMPONENT} ${MODULE})

    ENDIF (PKG_CONFIG_FOUND)

    FIND_PATH(FFmpeg_${COMPONENT}_INCLUDE_DIR
      NAMES lib${COMPONENT}/${COMPONENT}.h
      HINTS ${FFmpeg_SRCDIR}
            ${FFmpeg_${COMPONENT}_INCLUDE_DIRS}
      PATHS /usr/include
            /usr/local/include
            /opt/local/include
            /sw/include)

    FIND_LIBRARY(FFmpeg_${COMPONENT}_LIBRARY
      NAMES ${COMPONENT}
      HINTS ${FFmpeg_BUILDDIR}/lib${COMPONENT}
            ${FFmpeg_SRCDIR}/lib${COMPONENT}
            ${FFmpeg_${COMPONENT}_LIBRARY_DIRS}
      PATHS /usr/lib
            /usr/local/lib
            /opt/local/lib
            /sw/lib)
  ENDMACRO(FFmpeg_FIND)

  # When using uninstalled out of source build of ffmpeg this header ends up in
  # the build directory while the rest are in the source directory
  FIND_PATH(FFmpeg_avconfig_h_INCLUDE_DIR
    NAMES libavutil/avconfig.h
    HINTS ${FFmpeg_BUILDDIR}
          ${FFmpeg_SRCDIR}
          ${FFmpeg_avutil_INCLUDE_DIRS}
    PATHS /usr/include
          /usr/local/include
          /opt/local/include
          /sw/include)

  FFmpeg_FIND(avformat)
  FFmpeg_FIND(avcodec)
  FFmpeg_FIND(avutil)
  FFmpeg_FIND(swscale)

  SET(FFmpeg_PROCESS_INCLUDES
    FFmpeg_avformat_INCLUDE_DIR
    FFmpeg_avcodec_INCLUDE_DIR
    FFmpeg_avutil_INCLUDE_DIR
    FFmpeg_avconfig_h_INCLUDE_DIR
    FFmpeg_swscale_INCLUDE_DIR)
  SET(FFmpeg_PROCESS_LIBS
    FFmpeg_avformat_LIBRARY FFmpeg_avformat_LIBRARIES
    FFmpeg_avcodec_LIBRARY FFmpeg_avcodec_LIBRARIES
    FFmpeg_avutil_LIBRARY FFmpeg_avutil_LIBRARIES
    FFmpeg_swscale_LIBRARY FFmpeg_swscale_LIBRARIES)
  LIBFIND_PROCESS(FFmpeg)
endif (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)

IF (FFmpeg_FOUND)
  SET(FFmpeg_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")
ENDIF (FFmpeg_FOUND)
