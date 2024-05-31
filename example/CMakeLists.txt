# CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

find_package(PkgConfig REQUIRED)

pkg_check_modules(glfw3 REQUIRED IMPORTED_TARGET glfw3)
pkg_check_modules(assimp REQUIRED IMPORTED_TARGET assimp)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)


project(ChillEngine_example)
add_executable(ChillEngine_example)
target_sources(ChillEngine_example PUBLIC main.cpp)

# Debug
# target_compile_options(ChillEngine_example PUBLIC "-g")

# Libraries
add_subdirectory("chill_engine")

target_link_libraries(ChillEngine_example
	PUBLIC PkgConfig::glfw3
	PUBLIC glad
	PUBLIC window
	PUBLIC buffer_obj
	PUBLIC shaders
	PUBLIC figures
	PUBLIC camera
	PUBLIC app
	PUBLIC light)

target_include_directories(ChillEngine_example PUBLIC ./chill_engine/include)
