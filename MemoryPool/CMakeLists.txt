cmake_minimum_required(VERSION 3.15)
project(memorypool)
include_directories(${PROJECT_SOURCE_DIR}/C-11)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LISTS)

add_executable(test ${SRC_LISTS})