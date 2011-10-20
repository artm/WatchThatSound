# Try to find FFmpeg
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
# TODO
#
# - use *-uninstalled.pc files from the source directory if supplied
# - extend lists with LIST(APPEND ...)
# - other components
# - components selector
#
# Based on scripts by Andreas Schneider and Lasse Kärkkäinen:
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#  Modified for other libraries by Lasse Kärkkäinen <tronic>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#

SET(FFmpeg_SRCDIR CACHE PATH "Path to FFmpeg source directory")

IF (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
  # in cache already
  SET(FFmpeg_FOUND TRUE)
ELSE (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
  INCLUDE(LibFindMacros)

  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  FIND_PACKAGE(PkgConfig)

  MACRO(FFmpeg_FIND COMPONENT)
    IF (PKG_CONFIG_FOUND)
      PKG_CHECK_MODULES(_FFmpeg_${COMPONENT} lib${COMPONENT})
    ENDIF (PKG_CONFIG_FOUND)

    FIND_PATH(FFmpeg_${COMPONENT}_INCLUDE_DIR
      NAMES lib${COMPONENT}/${COMPONENT}.h
      HINTS ${FFmpeg_SRCDIR}
            ${_FFmpeg_${COMPONENT}_INCLUDE_DIRS} 
      PATHS /usr/include 
            /usr/local/include 
            /opt/local/include 
            /sw/include)

    FIND_LIBRARY(FFmpeg_${COMPONENT}_LIBRARY
      NAMES ${COMPONENT}
      HINTS ${FFmpeg_SRCDIR}/lib${COMPONENT}
            ${_FFmpeg_${COMPONENT}_LIBRARY_DIRS} 
      PATH /usr/lib 
            /usr/local/lib 
            /opt/local/lib 
            /sw/lib)
  ENDMACRO(FFmpeg_FIND)

  FFmpeg_FIND(avformat)
  FFmpeg_FIND(avcodec)
  FFmpeg_FIND(avutil)
  FFmpeg_FIND(swscale)

  SET(FFmpeg_PROCESS_INCLUDES 
    FFmpeg_avformat_INCLUDE_DIR
    FFmpeg_avcodec_INCLUDE_DIR
    FFmpeg_avutil_INCLUDE_DIR
    FFmpeg_swscale_INCLUDE_DIR)
  SET(FFmpeg_PROCESS_LIBS 
    FFmpeg_avformat_LIBRARY
    FFmpeg_avcodec_LIBRARY
    FFmpeg_avutil_LIBRARY
    FFmpeg_swscale_LIBRARY)
  LIBFIND_PROCESS(FFmpeg)
endif (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)

IF (FFmpeg_FOUND)
  SET(FFmpeg_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")
ENDIF (FFmpeg_FOUND)
