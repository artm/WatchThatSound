SET(Portaudio_SRCDIR CACHE PATH "Path to Portaudio source directory")

IF (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
  # in cache already
  SET(Portaudio_FOUND TRUE)
ELSE (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
  INCLUDE(LibFindMacros)

  FIND_PATH(Portaudio_INCLUDE_DIR portaudio.h
    HINTS ${Portaudio_INCLUDEDIR} ${Portaudio_SRCDIR}/include)

  FIND_LIBRARY(Portaudio_LIBRARY
    NAMES portaudio
    HINTS ${Portaudio_LIBDIR} ${Portaudio_SRCDIR}/lib/.libs )

  # TODO make this automatic (parse .pc or .la)
  SET(Portaudio_DEPENDS -lwinmm)

  SET(Portaudio_PROCESS_INCLUDES Portaudio_INCLUDE_DIR)
  SET(Portaudio_PROCESS_LIBS Portaudio_LIBRARY Portaudio_DEPENDS)
  LIBFIND_PROCESS(Portaudio)

ENDIF (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
