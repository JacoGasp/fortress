cmake_minimum_required(VERSION 3.16)

project(Fortress)

set(REQUIRED_LIBS Core Gui Qml Quick Widgets)
set(REQUIRED_LIBS_QUALIFIED Qt6::Core Qt6::Gui Qt6::Qml Qt::Quick Qt::Widgets)

find_package(Qt6 COMPONENTS ${REQUIRED_LIBS} REQUIRED)


add_subdirectory(src)