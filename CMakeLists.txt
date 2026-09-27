cmake_minimum_required(VERSION 3.15)
project(PaintLikeGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(paint src/main.cpp)

target_link_libraries(paint PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
