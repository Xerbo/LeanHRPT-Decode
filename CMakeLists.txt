cmake_minimum_required(VERSION 3.0.0)
project(LeanHRPT-Decode CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(tools)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE CXX_SOURCE_FILES src/*.cpp)
IF (WIN32)
add_executable(LeanHRPT-Decode WIN32 ${CXX_SOURCE_FILES})
ELSE()
add_executable(LeanHRPT-Decode ${CXX_SOURCE_FILES})
ENDIF()

set_property(TARGET LeanHRPT-Decode PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET LeanHRPT-Decode PROPERTY CXX_STANDARD 14)

target_include_directories(LeanHRPT-Decode PUBLIC src)

target_compile_options(LeanHRPT-Decode PRIVATE -Wall -Wextra -Wpedantic)

if (USE_ADDRESS_SANITIZER)
    target_compile_options(LeanHRPT-Decode PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(LeanHRPT-Decode PRIVATE -fsanitize=address)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
target_link_libraries(LeanHRPT-Decode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(LeanHRPT-Decode PRIVATE Qt${QT_VERSION_MAJOR}::Network)

find_library(MUPARSER_PATH NAMES muparser NO_CACHE REQUIRED)
target_link_libraries(LeanHRPT-Decode PUBLIC ${MUPARSER_PATH})
find_library(LIBPREDICT_PATH NAMES predict NO_CACHE REQUIRED)
target_link_libraries(LeanHRPT-Decode PUBLIC ${LIBPREDICT_PATH})
find_library(SHAPELIB_PATH NAMES shp NO_CACHE REQUIRED)
target_link_libraries(LeanHRPT-Decode PUBLIC ${SHAPELIB_PATH})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(LeanHRPT-Decode PRIVATE OpenMP::OpenMP_CXX)
endif()

# Get version from git
find_package(Git)
if (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tag WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} OUTPUT_VARIABLE GIT_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(VERSION "${GIT_TAG}")
else()
    set(VERSION "Unknown")
endif()
target_compile_definitions(LeanHRPT-Decode PRIVATE "VERSION=\"${VERSION}\"")

# Binaries
install(TARGETS LeanHRPT-Decode RUNTIME DESTINATION bin)
# Desktop file and icon
install(FILES ${CMAKE_SOURCE_DIR}/LeanHRPT-Decode.desktop DESTINATION share/applications)
install(FILES ${CMAKE_SOURCE_DIR}/logo128.png DESTINATION share/icons/hicolor/128x128/apps RENAME LeanHRPT-Decode.png)
install(FILES ${CMAKE_SOURCE_DIR}/logo.svg DESTINATION share/icons/hicolor/scalable/apps RENAME LeanHRPT-Decode.svg)
configure_file(logo128.png logo128.png COPYONLY)
configure_file(logo.svg logo.svg COPYONLY)
# Config files
install(FILES ${CMAKE_SOURCE_DIR}/projection.ini DESTINATION share/leanhrpt)
install(FILES ${CMAKE_SOURCE_DIR}/calibration.ini DESTINATION share/leanhrpt)
install(FILES ${CMAKE_SOURCE_DIR}/presets.ini DESTINATION share/leanhrpt)
install(FILES ${CMAKE_SOURCE_DIR}/gradients.ini DESTINATION share/leanhrpt)
configure_file(projection.ini projection.ini COPYONLY)
configure_file(calibration.ini calibration.ini COPYONLY)
configure_file(presets.ini presets.ini COPYONLY)
configure_file(gradients.ini gradients.ini COPYONLY)

# Package information
string(SUBSTRING ${VERSION} 1 -1 CPACK_PACKAGE_VERSION)
set(CPACK_PACKAGE_NAME "LeanHRPT-Decode")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An easy to use HRPT decoder")
set(CPACK_PACKAGE_DESCRIPTION "LeanHRPT is an easy to use and powerful tool for the manipulation of Level 0 HRPT data.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Xerbo/LeanHRPT-Decode")
set(CPACK_PACKAGE_CONTACT "Xerbo (xerbo@protonmail.com)")
# Debian
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "gdal-bin")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpredict")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

# Only allow packaging if we know what we are packaging
if (NOT VERSION STREQUAL "Unknown")
    include(CPack)
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen ALL
                      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating documentation with Doxygen"
                      VERBATIM)
endif()
