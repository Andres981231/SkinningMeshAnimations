cmake_minimum_required(VERSION 3.0.0)
project(skinning_mesh_animations VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)
#add opengl
find_package(OpenGL)
# add glfw
include_directories(3rdLibs/glfw-3.3.2/include)
link_directories(3rdLibs/lib)
# add glad
include_directories(3rdLibs/glad/include)
add_library(glad 3rdLibs/glad/src/glad.c)
# Add Include Path
include_directories(
	src
	head
	include
)
# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	src/*.c
    src/*.cpp
    head/*.hpp
	src/*.cc)
if(WIN32)
add_definitions(-D_AMD64_)
endif()
message("Following files are complied " ${SOURCE_FILES})
add_executable(skinning_mesh_animations ${SOURCE_FILES})
target_link_libraries(skinning_mesh_animations glad glfw3 ${OPENGL_glu_LIBRARY} ${OPENGL_LIBRARIES})