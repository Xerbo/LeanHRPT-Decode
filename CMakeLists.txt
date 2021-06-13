cmake_minimum_required (VERSION 3.0.0)
project(LeanHRPT-Decode CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
target_link_libraries(LeanHRPT-Decode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Bodge fix for GitHub Actions
target_link_directories(LeanHRPT-Decode PRIVATE /tmp/build/lib)

target_link_libraries(LeanHRPT-Decode PUBLIC correct)
