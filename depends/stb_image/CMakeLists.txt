cmake_minimum_required(VERSION 3.16)
project(stb_image)

add_library(stb_image include/stb_image.h include/stb_image_write.h stb_image_impl.c)
target_include_directories(stb_image PUBLIC include/)