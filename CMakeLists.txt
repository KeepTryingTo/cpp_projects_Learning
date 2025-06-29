cmake_minimum_required(VERSION 3.15)

project(demoGoogleTest)

set(CMAKE_CXX_STANDARD 17)
set(LIB_DIR ${CMAKE_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${LIB_DIR}/lib)

add_executable(test ${SRC_LIST})

target_compile_options(test PRIVATE -pthread)
target_link_libraries(test  PRIVATE -pthread libgtest_main.a libgtest.a)