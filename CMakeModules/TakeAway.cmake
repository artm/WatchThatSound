# Find 'non-standard' dll's linked against us and add them to install target
# This will make the CPack to add them to the installer

MACRO(COLLECT_TAKEAWAY _TAKEAWAY_LIBS)

  IF(WIN32)
    # expand the argument
    SET(TAKEAWAY_LIBS ${${_TAKEAWAY_LIBS}})

    IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
      SET(SKIP_TAG "optimized")
    ELSE(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
      SET(SKIP_TAG "debug")
    ENDIF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

    SET(SKIP_NEXT false)
    MESSAGE(STATUS "Looking for DLLs to take away")
    FOREACH(LIB ${TAKEAWAY_LIBS} "libgcc_s_dw2-1" "libstdc++-6" "mingwm10")
      IF(SKIP_NEXT)
        SET(SKIP_NEXT false)
      ELSEIF(${LIB} STREQUAL ${SKIP_TAG})
        SET(SKIP_NEXT true)
      ELSEIF(NOT ${LIB} STREQUAL "debug"
          AND NOT ${LIB} STREQUAL "optimized"
          AND NOT ${LIB} STREQUAL "general")

        STRING(REGEX REPLACE "^-l" "" LIB ${LIB})
        STRING(REGEX REPLACE "\\.a$" "" LIB ${LIB})
        STRING(REGEX REPLACE "^.*[/\\]lib" "" LIB ${LIB})
        FIND_FILE(DLL ${LIB}.dll HINTS ${QT_BINARY_DIR} ENV "PATH" NO_DEFAULT_PATH)
        IF(DLL)
          IF(NOT DLL MATCHES ":[/\\]Windows[/\\]")
            MESSAGE(STATUS " [+] ${LIB} => ${DLL}")
            LIST(APPEND TAKEAWAY_DLLS ${DLL})
          ELSE(NOT DLL MATCHES ":[/\\]Windows[/\\]")
            MESSAGE(STATUS " [ ] Ignoring standard dll for ${LIB} (${DLL})")
          ENDIF(NOT DLL MATCHES ":[/\\]Windows[/\\]")
          UNSET(DLL CACHE)
        ELSE(DLL)
          MESSAGE(STATUS " [-] No dll for ${LIB} found")
          MESSAGE(STATUS "     (probably static, investigate if .exe complains)")
        ENDIF(DLL)

      ENDIF(SKIP_NEXT)
    ENDFOREACH(LIB ${TAKEAWAY_LIBS})
    LIST(REMOVE_DUPLICATES TAKEAWAY_DLLS)
    MESSAGE(STATUS "Take away DLLs: ${TAKEAWAY_DLLS}")

    INSTALL(
      FILES ${TAKEAWAY_DLLS}
      DESTINATION bin)
  ENDIF(WIN32)

ENDMACRO(COLLECT_TAKEAWAY TAKEAWAY_LIBS)

MACRO(COLLECT_TAKEAWAY_QT_PLUGINS _PLUGIN_DIRS)
  SET(PLUGIN_DIRS ${${_PLUGIN_DIRS}})
  IF(WIN32)
    FOREACH(DIR ${PLUGIN_DIRS})
      IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        FILE(GLOB DLLS "${QT_PLUGINS_DIR}/${DIR}/*d4.dll")
      ELSE(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        FILE(GLOB ALL_DLLS "${QT_PLUGINS_DIR}/${DIR}/*.dll")
        # filter out debug versions heuristically
        FOREACH(DLL ${ALL_DLLS})
          IF(DLL MATCHES "d4\\.dll$")
            STRING(REGEX REPLACE "d4\\.dll$" "4.dll" DLL_NO_D ${DLL})
            LIST(FIND ALL_DLLS ${DLL_NO_D} IDX_NO_D)
            IF(IDX_NO_D LESS 0)
              # assume this is a non-debug library with a name ending in d
              LIST(APPEND DLLS ${DLL})
            ENDIF(IDX_NO_D LESS 0)
          ELSE(DLL MATCHES "d4\\.dll$")
            LIST(APPEND DLLS ${DLL})
          ENDIF(DLL MATCHES "d4\\.dll$")
        ENDFOREACH(DLL ${ALL_DLLS})
      ENDIF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
      INSTALL(
        FILES ${DLLS}
        DESTINATION bin/plugins/${DIR})
      LIST(APPEND TAKEAWAY_PLUGINS ${DLLS})
    ENDFOREACH(DIR ${PLUGIN_DIRS})
    MESSAGE(STATUS "Take away plugins: ${TAKEAWAY_PLUGINS}")
  ENDIF(WIN32)
ENDMACRO(COLLECT_TAKEAWAY_QT_PLUGINS PLUGIN_DIRS)
