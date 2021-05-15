cmake_minimum_required(VERSION 3.17)

project(Fortress)


set(REQUIRED_LIBS Core Gui Qml Quick)
set(REQUIRED_LIBS_QUALIFIED Qt6::Core Qt6::Gui Qt6::Qml Qt::Quick)

find_package(Qt6 COMPONENTS ${REQUIRED_LIBS} REQUIRED)


add_subdirectory(src)