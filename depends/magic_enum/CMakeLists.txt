cmake_minimum_required(VERSION 3.14)
project(
  magic_enum
  VERSION "0.9.7"
  HOMEPAGE_URL "https://github.com/Neargye/magic_enum"
  DESCRIPTION
    "A library that provides static reflection for enums, work with any enum type without any macro or boilerplate code."
  LANGUAGES CXX
)

add_library(magic_enum INTERFACE include/magic_enum.hpp)
target_include_directories(magic_enum INTERFACE include/)