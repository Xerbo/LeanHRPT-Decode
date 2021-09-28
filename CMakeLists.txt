cmake_minimum_required (VERSION 3.0.0)
project(LeanHRPT-Decode CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE CXX_SOURCE_FILES src/*.cpp)
add_executable(LeanHRPT-Decode ${CXX_SOURCE_FILES})

set_property(TARGET LeanHRPT-Decode PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET LeanHRPT-Decode PROPERTY CXX_STANDARD 11)

target_include_directories(LeanHRPT-Decode PUBLIC src)

target_compile_options(LeanHRPT-Decode PRIVATE -Wall -Wextra -Wpedantic)

if (USE_ADDRESS_SANITIZER)
    target_compile_options(LeanHRPT-Decode PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(LeanHRPT-Decode PRIVATE -fsanitize=address)
endif()

# If you CPU doesn't have SSE 4.2 then get a newer CPU
target_compile_options(LeanHRPT-Decode PRIVATE -msse4.2)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
target_link_libraries(LeanHRPT-Decode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(LeanHRPT-Decode PRIVATE Qt${QT_VERSION_MAJOR}::Network)

# Bodge fix for GitHub Actions
if (WAR_CRIMES)
    target_link_directories(LeanHRPT-Decode PRIVATE /tmp/build/lib)
    target_link_directories(LeanHRPT-Decode PRIVATE /tmp/winbuild/lib)
endif()

target_link_libraries(LeanHRPT-Decode PUBLIC correct)
target_link_libraries(LeanHRPT-Decode PUBLIC muparser)
target_link_libraries(LeanHRPT-Decode PUBLIC predict)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(LeanHRPT-Decode PRIVATE OpenMP::OpenMP_CXX)
endif()

find_package(Git)
if (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tag WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} OUTPUT_VARIABLE GIT_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
    target_compile_definitions(LeanHRPT-Decode PRIVATE "VERSION=\"${GIT_TAG}\"")
else()
    target_compile_definitions(LeanHRPT-Decode PRIVATE "VERSION=\"Unknown\"")
endif()
