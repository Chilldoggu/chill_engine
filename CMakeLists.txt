# CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23) 

project(OpenGL_proj)
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC "main.cpp") 
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/install")

# g++ debug mode
if (CMAKE_CXX_COMPILER_ID STREQUAL "g++")
	target_compile_options(${PROJECT_NAME} PUBLIC "-g")
endif ()


# Libraries
add_subdirectory("_chill_renderer")

add_library(chill_scene STATIC "scene.cpp")
target_include_directories(chill_scene PUBLIC "_chill_renderer/include")
target_link_libraries(chill_scene PUBLIC chill_renderer)

target_link_libraries(${PROJECT_NAME} PUBLIC chill_renderer chill_scene)
