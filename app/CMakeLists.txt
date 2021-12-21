set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fstrict-aliasing -pedantic-errors -pedantic -Wno-deprecated-declarations -Wno-unused-variable")


# Setup Qt
set(REQUIRED_LIBS Core Gui Qml Quick Widgets Charts)
set(REQUIRED_LIBS_QUALIFIED Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Widgets Qt6::Charts)

find_package(Qt6 COMPONENTS ${REQUIRED_LIBS} REQUIRED)

qt_add_resources(QT_RESOURCES ../qml/qml.qrc)

# Setup executable

file(GLOB INCLUDES ../include/*.h)
set(SOURCES ../src/Backend.cpp ../src/ChartModel.cpp)

if (APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp ${QT_RESOURCES} ${SOURCES} ${INCLUDES})
else()
    add_executable(${PROJECT_NAME} main.cpp ${QT_RESOURCES} ${SOURCES} ${INCLUDES})
endif()

target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
target_include_directories(${PROJECT_NAME} PRIVATE ../include)