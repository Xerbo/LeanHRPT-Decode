cmake_minimum_required(VERSION 3.0.0)
project(LeanHRPT-Tools CXX)

add_executable(vcdu2cadu vcdu2cadu.cpp)

add_executable(bin2raw16 bin2raw16.cpp ${CMAKE_SOURCE_DIR}/src/protocol/deframer.cpp ${CMAKE_SOURCE_DIR}/src/protocol/repack.cpp)
target_include_directories(bin2raw16 PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_executable(bin2cadu bin2cadu.cpp ${CMAKE_SOURCE_DIR}/src/protocol/ccsds/deframer.cpp)
target_include_directories(bin2cadu PUBLIC ${CMAKE_SOURCE_DIR}/src)
