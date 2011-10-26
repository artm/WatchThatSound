SET(Portaudio_SRCDIR CACHE PATH "Path to Portaudio source directory")

IF (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
  # in cache already
  SET(Portaudio_FOUND TRUE)
ELSE (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
  FIND_PACKAGE(PkgConfig)

  IF (PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(PKG_Portaudio portaudio-2.0)
  ENDIF (PKG_CONFIG_FOUND)

  INCLUDE(LibFindMacros)

  FIND_PATH(Portaudio_INCLUDE_DIR 
    NAMES portaudio.h
    HINTS 
    ${PKG_Portaudio_INCLUDE_DIRS}
    ${Portaudio_SRCDIR}/include)

  FIND_LIBRARY(Portaudio_LIBRARY
    NAMES portaudio
    HINTS 
    ${PKG_Portaudio_LIBRARY_DIRS} 
    ${Portaudio_SRCDIR}/lib/.libs )

  SET(Portaudio_PROCESS_INCLUDES Portaudio_INCLUDE_DIR)
  SET(Portaudio_PROCESS_LIBS Portaudio_LIBRARY)
  LIBFIND_PROCESS(Portaudio)

ENDIF (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
