cmake_minimum_required(VERSION 3.10)
project(STLViewer)

set(CMAKE_CXX_STANDARD 17)

find_package(wxWidgets REQUIRED gl base core)
include(${wxWidgets_USE_FILE})

find_package(OpenGL REQUIRED)

add_executable(stl_viewer src/main.cpp src/stl_loader.cpp src/gl_canvas.cpp)
target_link_libraries(stl_viewer ${wxWidgets_LIBRARIES} OpenGL::GL GLU)

