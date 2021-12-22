set(CMAKE_CXX_STANDARD 20)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "-pthread")
endif(UNIX AND NOT APPLE)

file(GLOB INCLUDES ../include/*.h)

add_executable(Server main.cpp ${INCLUDES} ../include/FRServer.h ../src/FRServer.cpp)
target_include_directories(Server PRIVATE ../include)
