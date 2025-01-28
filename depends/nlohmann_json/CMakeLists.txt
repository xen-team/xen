cmake_minimum_required(VERSION 3.14)
project(json)

add_library(json INTERFACE include/json.hpp)
target_include_directories(json INTERFACE include/)