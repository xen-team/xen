cmake_minimum_required(VERSION 3.15.7)
project(
  EnTT
  LANGUAGES C CXX
)

add_library(EnTT INTERFACE include/entt.hpp)
target_include_directories(EnTT INTERFACE include/)