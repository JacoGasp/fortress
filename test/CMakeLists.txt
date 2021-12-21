set(CMAKE_CXX_STANDARD 20)
file(GLOB INCLUDES ../include/**/*.h)



add_executable(Test main.cpp ${INCLUDES})
target_include_directories(Test PRIVATE ../include)
if(APPLE)
    target_include_directories(Test PUBLIC /usr/local/Cellar/asio/current/include)
endif(APPLE)
