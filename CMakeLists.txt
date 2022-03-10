cmake_minimum_required(VERSION 3.13)
project(sandy)

set(CMAKE_CXX_STANDARD 14)
# depending on the name of the glfw folder
set(GLFW glfw-3.3)

# required libraries in libs directory
add_subdirectory(libs/${GLFW})
include_directories(libs/${GLFW}/include)
include_directories(libs/glad/include)
include_directories(libs/glm)

set(SRC src/main.cpp src/shader.h)

add_executable(sand ${SRC} libs/glad/src/glad.c)
target_link_libraries(sand glfw ${GLFW_LIBRARIES})
