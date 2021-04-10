cmake_minimum_required (VERSION 3.0.0)
project(LeanHRPT-Decode CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE CXX_SOURCE_FILES src/*.cpp)
add_executable(LeanHRPT-Decode ${CXX_SOURCE_FILES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

target_include_directories(LeanHRPT-Decode PUBLIC src)

target_compile_options(LeanHRPT-Decode PRIVATE -Wall -Wextra -pedantic)

# If you CPU doesn't have SSE 4.2 then get a newer CPU
target_compile_options(LeanHRPT-Decode PRIVATE -msse4.2)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
target_link_libraries(LeanHRPT-Decode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(LeanHRPT-Decode PUBLIC correct)
