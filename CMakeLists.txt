cmake_minimum_required(VERSION 3.28)

project(chill_engine)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23) 
set(IMGUI_DIR "include/imgui/")
set(IMGUI_BACKEND_DIR "include/imgui/backend/")
set(SRC "src")
if (WIN32)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif (WIN32)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

if (UNIX)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(glfw3 REQUIRED IMPORTED_TARGET glfw3)
	pkg_check_modules(assimp REQUIRED IMPORTED_TARGET assimp)
endif (UNIX)

if (WIN32) 
	add_library(glfw3 SHARED IMPORTED)
	set_target_properties(glfw3 PROPERTIES
		IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/lib/glfw3.dll"
		IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/lib/glfw3dll.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include/GLFW"
	) 

	add_library(assimp SHARED IMPORTED)
	set_target_properties(assimp PROPERTIES
		IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/lib/assimp-vc143-mtd.dll"
		IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/lib/assimp-vc143-mtd.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include/assimp"
	) 
endif (WIN32)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
	"${SRC}/app.cpp" 
	"${SRC}/assert.cpp" 
	"${SRC}/camera.cpp" 
	"${SRC}/window.cpp" 
	"${SRC}/shaders.cpp" 
	"${SRC}/meshes.cpp" 
	"${SRC}/model.cpp" 
	"${SRC}/light.cpp" 
)
target_include_directories(${PROJECT_NAME} PUBLIC "include")

add_library(glad STATIC ./src/glad.c)

add_library(imgui STATIC)
target_sources(imgui PUBLIC
	"${IMGUI_DIR}/imgui.cpp"
	"${IMGUI_DIR}/imgui_demo.cpp"
	"${IMGUI_DIR}/imgui_draw.cpp"
	"${IMGUI_DIR}/imgui_tables.cpp"
	"${IMGUI_DIR}/imgui_widgets.cpp"
	"${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp"
	"${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp"
)

target_include_directories(glad         PUBLIC "include")
target_include_directories(imgui        PUBLIC ${IMGUI_DIR} ${IMGUI_BACKEND_DIR} "include")

if (UNIX)
	target_link_libraries(chill_engine PUBLIC 
		glad 
		imgui 
		PkgConfig::glfw3 
		PkgConfig::assimp
	)
endif (UNIX)

if (WIN32)
	target_link_libraries(chill_engine PUBLIC 
		glad 
		imgui 
		glfw3 
		assimp
	)
endif (WIN32)
