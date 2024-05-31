find_package(PkgConfig REQUIRED)
pkg_check_modules(glfw3 REQUIRED IMPORTED_TARGET glfw3)

set(IMGUI_DIR ./include/imgui/)
set(IMGUI_BACKEND_DIR ./include/imgui/backend/)

add_library(app         ./src/app.cpp)
add_library(glad        ./src/glad.c)
add_library(assert      ./src/assert.cpp)
add_library(camera      ./src/camera.cpp)
add_library(window      ./src/window.cpp)
add_library(shaders     ./src/shaders.cpp)
add_library(figures     ./src/figures.cpp)
add_library(buffer_obj  ./src/buffer_obj.cpp)
add_library(light       ./src/light.cpp)
add_library(imgui STATIC
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
	${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
	)

target_include_directories(glad        PUBLIC "include")
target_include_directories(app         PUBLIC "include")
target_include_directories(light       PUBLIC "include")
target_include_directories(camera      PUBLIC "include")
target_include_directories(assert      PUBLIC "include")
target_include_directories(window      PUBLIC "include")
target_include_directories(shaders     PUBLIC "include")
target_include_directories(figures     PUBLIC "include")
target_include_directories(buffer_obj  PUBLIC "include")
target_include_directories(imgui       PUBLIC ${IMGUI_DIR} ${IMGUI_BACKEND_DIR})

target_link_libraries(camera         PUBLIC PkgConfig::glfw3 glad)
target_link_libraries(shaders        PUBLIC PkgConfig::glfw3 glad)
target_link_libraries(buffer_obj     PUBLIC PkgConfig::glfw3 glad)
target_link_libraries(figures        PUBLIC PkgConfig::glfw3 glad buffer_obj)
target_link_libraries(window         PUBLIC PkgConfig::glfw3 glad assert imgui)
target_link_libraries(app            PUBLIC PkgConfig::glfw3 glad camera window)
