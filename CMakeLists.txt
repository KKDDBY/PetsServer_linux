cmake_minimum_required(VERSION 3.20)
project(PETS_SERVER)

SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -L/usr/lib64/mysql -DKKDD_TEST")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(${CMAKE_SOURCE_DIR} DIR_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/ObjectModule DIR_SRCS2)
aux_source_directory(${CMAKE_SOURCE_DIR}/MySqlModule DIR_SRCS3)

add_executable(server main.cpp ${DIR_SRCS} ${DIR_SRCS2} ${DIR_SRCS3})

#include_directories()#添加头文件的搜索路径

target_include_directories(server
    PUBLIC ${CMAKE_SOURCE_DIR}/muduo/include
    PUBLIC ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/NetMsg
    PUBLIC ${CMAKE_SOURCE_DIR}/ObjectModule
    PUBLIC ${CMAKE_SOURCE_DIR}/MySqlModule)



target_link_libraries(server muduo_net  muduo_base mysqlclient pthread)




