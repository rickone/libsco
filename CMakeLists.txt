cmake_minimum_required(VERSION 3.0)
project(sco)
aux_source_directory(src SRC)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-D_DEBUG -D_XOPEN_SOURCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(sco SHARED ${SRC})

find_package(LIBEVENT REQUIRED)
if(LIBEVENT_FOUND)
    include_directories(${LIBEVENT_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "libevent not found!")
endif()
target_link_libraries(sco pthread dl event)

add_executable(hello ${CMAKE_CURRENT_SOURCE_DIR}/test/hello.cpp)
target_link_libraries(hello pthread dl event sco)

add_executable(mutex ${CMAKE_CURRENT_SOURCE_DIR}/test/mutex.cpp)
target_link_libraries(mutex pthread dl event sco)

add_executable(sleep ${CMAKE_CURRENT_SOURCE_DIR}/test/sleep.cpp)
target_link_libraries(sleep pthread dl event sco)

add_executable(wait ${CMAKE_CURRENT_SOURCE_DIR}/test/wait.cpp)
target_link_libraries(wait pthread dl event sco)

add_executable(wait_group ${CMAKE_CURRENT_SOURCE_DIR}/test/wait_group.cpp)
target_link_libraries(wait_group pthread dl event sco)

install(TARGETS sco LIBRARY DESTINATION lib)