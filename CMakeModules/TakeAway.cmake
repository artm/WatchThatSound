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
