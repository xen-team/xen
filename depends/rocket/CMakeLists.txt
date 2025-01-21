cmake_minimum_required(VERSION 3.2)
project(rocket CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rocket INTERFACE include/rocket.hpp)
target_include_directories(rocket INTERFACE include/)