cmake_minimum_required(VERSION 3.6)
project(imgui)

add_library(imgui STATIC
	include/imgui/imgui.h
	include/imgui/imgui_internal.h
	include/imgui/imgui_extension.hpp
	include/imgui/imgui_impl_opengl3.h
	include/imgui/imgui_impl_glfw.h

	src/imgui.cpp
	src/imgui_draw.cpp
	src/imgui_widgets.cpp
	src/imstb_rectpack.h
	src/imstb_truetype.h
	src/imgui_tables.cpp
	src/imgui_demo.cpp
	src/imgui_impl_glfw.cpp
	src/imgui_impl_opengl3.cpp
	src/imgui_stdlib.h
	src/imgui_stdlib.cpp
	src/imgui_extension.cpp
)

target_include_directories(imgui PUBLIC include/)