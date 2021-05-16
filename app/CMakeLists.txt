set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fstrict-aliasing -pedantic-errors -pedantic -Wno-deprecated-declarations -Wno-unused-variable")

# Setup Qt
set(REQUIRED_LIBS Core Gui Qml Quick Widgets)
set(REQUIRED_LIBS_QUALIFIED Qt6::Core Qt6::Gui Qt6::Qml Qt::Quick Qt::Widgets)

find_package(Qt6 COMPONENTS ${REQUIRED_LIBS} REQUIRED)

qt_add_resources(QT_RESOURCES ../qml/qml.qrc)

# Setup executable

file(GLOB INCLUDES ../include/*.h)
file(GLOB SOURCES ../src/*.cpp)

add_executable(${PROJECT_NAME} main.cpp ${QT_RESOURCES} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
target_include_directories(${PROJECT_NAME} PRIVATE ../include)