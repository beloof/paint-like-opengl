cmake_minimum_required(VERSION 3.15)
project(PaintLikeGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(paint     
    src/Canvas.cpp
    src/Brush.cpp
    src/main.cpp
)

target_link_libraries(paint PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_include_directories(paint PRIVATE ${CMAKE_SOURCE_DIR}/third_party/stb)

add_custom_command(TARGET paint POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:paint>/assets)