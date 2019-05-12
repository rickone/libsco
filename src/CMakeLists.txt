aux_source_directory(. SRC)

add_library(sco SHARED ${SRC})

find_package(LIBEVENT REQUIRED)
if(LIBEVENT_FOUND)
    message("Libevent ${LIBEVENT_VERSION} found!")
    include_directories(${LIBEVENT_INCLUDE_DIRS})
    target_link_libraries(sco ${LIBEVENT_LIBRARIES})
else()
    message(FATAL_ERROR "libevent not found!")
endif()

target_link_libraries(sco pthread dl)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -g -Wall)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++14)
add_definitions(-D_DEBUG -D_XOPEN_SOURCE)