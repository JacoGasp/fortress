find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(Backend)
target_sources(Backend PRIVATE backend.cpp ../include/Backend.h)
target_link_libraries(Qt6::Core)