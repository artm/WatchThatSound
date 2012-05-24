SET(PROJECT_NAME_LONG "Watch That Sound Tool")
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "17")
SET(PROJECT_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" CACHE STRING "Version" FORCE)
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(APPLICATION_NAME ${PROJECT_NAME})
SET(PROJECT_UPDATE_CHANNEL "https://ftp.v2.nl/~artm/WTS3/devel.xml" CACHE STRING "Update channel URL")
IF(WIN32)
  SET(CPACK_GENERATOR NSIS)
  SET(CPACK_PACKAGE_FILE_NAME "Setup-${PROJECT_NAME}-${PROJECT_VERSION}")
ELSE(WIN32)
  SET(CPACK_GENERATOR DragNDrop)
  SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
ENDIF(WIN32)

SET(CPACK_PACKAGE_VENDOR "Stichting Watch That Sound / V2_ Lab")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "A soundtrack composition worksop tool developed by V2_ Lab for Watch That Sound foundation")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME_LONG})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
# TODO convert markdown to html
CONFIGURE_FILE(README.md README.txt)
SET(CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/Readme.txt")
SET(CPACK_MONOLITHIC_INSTALL true)
SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME};${PROJECT_NAME_LONG}")
SET(CPACK_STRIP_FILES true)
SET(CPACK_DMG_VOLUME_NAME ${APPLICATION_NAME}-${PROJECT_VERSION})
SET(CPACK_DMG_FORMAT UDBZ)
INCLUDE(CPack)


