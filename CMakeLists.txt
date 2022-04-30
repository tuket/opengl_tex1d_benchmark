cmake_minimum_required(VERSION 3.8)
project(tex1d_benchmark)

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/glad)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

add_executable(tex1d_benchmark
	src/main.cpp
	src/utils.hpp src/utils.cpp
)
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tex1d_benchmark)
target_link_libraries(tex1d_benchmark
	glad
	glfw
	glm
)
