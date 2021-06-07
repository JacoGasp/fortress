set(CMAKE_CXX_STANDARD 20)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "-pthread")
endif(UNIX AND NOT APPLE)

if(APPLE)
    include_directories(/usr/local/Cellar/asio/1.18.2/include)
endif(APPLE)
# Setup executable

file(GLOB INCLUDES ../include/*.h)
file(GLOB SOURCES ../src/*.cpp)


add_executable(SimpleServer SimpleServer.cpp ${INCLUDES})
add_executable(SimpleClient SimpleClient.cpp ${INCLUDES})

target_include_directories(SimpleServer PRIVATE ../include)
target_include_directories(SimpleClient PRIVATE ../include)