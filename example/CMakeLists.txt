# CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

find_package(PkgConfig REQUIRED)

pkg_check_modules(glfw3 REQUIRED IMPORTED_TARGET glfw3)
pkg_check_modules(assimp REQUIRED IMPORTED_TARGET assimp)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)


project(OpenGL_lightning)
add_executable(OpenGL_lightning)
target_sources(OpenGL_lightning PUBLIC main.cpp)

# For Debug possibility
target_compile_options(OpenGL_lightning PUBLIC "-g")

# Libraries
add_subdirectory("chill_engine")

target_link_libraries(OpenGL_lightning 
	PUBLIC PkgConfig::glfw3
	PUBLIC PkgConfig::assimp
	PUBLIC glad
	PUBLIC window
	PUBLIC buffer_obj
	PUBLIC shaders
	PUBLIC figures
	PUBLIC camera
	PUBLIC app
	PUBLIC light
	PUBLIC imgui)

target_include_directories(OpenGL_lightning PUBLIC ./chill_engine/include)
