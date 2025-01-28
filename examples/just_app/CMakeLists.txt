cmake_minimum_required (VERSION 3.16)

project (xen-just-app-example)

add_executable(xen-just-app-example main.cpp)
target_link_libraries(${PROJECT_NAME} xen)