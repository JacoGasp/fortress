cmake_minimum_required(VERSION 3.16)

project(Fortress)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX AND NOT APPLE)

add_subdirectory(app)
add_subdirectory(server)
add_subdirectory(networking_examples)
add_subdirectory(test)