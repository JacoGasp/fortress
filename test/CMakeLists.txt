set(CMAKE_CXX_STANDARD 20)
file(GLOB INCLUDES ../include/**/*.h)

if(APPLE)
    include_directories(/usr/local/Cellar/asio/1.18.2/include)
endif(APPLE)

add_executable(Test main.cpp ${INCLUDES})
target_include_directories(Test PRIVATE ../include)