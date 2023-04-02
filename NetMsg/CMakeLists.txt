cmake_minimum_required(VERSION 3.20)

project(PETSSERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_CURRENT_LIST_DIR}/NetMsg ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

aux_source_directory(./ DIR_SRCS)

link_directories(${CMAKE_SOURCE_DIR}/muduo/lib)
add_executable(PETS_SERVER main.cpp ${DIR_SRCS})

target_link_libraries(PETS_SERVER muduo_net muduo_base pthread)
