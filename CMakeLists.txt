find_package(PkgConfig REQUIRED)
pkg_check_modules(glfw3 REQUIRED IMPORTED_TARGET glfw3)
pkg_check_modules(assimp REQUIRED IMPORTED_TARGET assimp)

set(IMGUI_DIR ./include/imgui/)
set(IMGUI_BACKEND_DIR ./include/imgui/backend/)

add_library(app         ./src/app.cpp)
add_library(glad        ./src/glad.c)
add_library(assert      ./src/assert.cpp)
add_library(camera      ./src/camera.cpp)
add_library(window      ./src/window.cpp)
add_library(shaders     ./src/shaders.cpp)
add_library(meshes      ./src/meshes.cpp)
add_library(model       ./src/model.cpp)
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

target_include_directories(app         PUBLIC "include")
target_include_directories(glad        PUBLIC "include")
target_include_directories(light       PUBLIC "include")
target_include_directories(camera      PUBLIC "include")
target_include_directories(assert      PUBLIC "include")
target_include_directories(window      PUBLIC "include")
target_include_directories(shaders     PUBLIC "include")
target_include_directories(meshes      PUBLIC "include")
target_include_directories(model       PUBLIC "include")
target_include_directories(imgui       PUBLIC ${IMGUI_DIR} ${IMGUI_BACKEND_DIR} "include")

target_link_libraries(model          PUBLIC meshes)
target_link_libraries(window         PUBLIC assert imgui)
target_link_libraries(app            PUBLIC window)
