cmake_minimum_required(VERSION 3.10)
project(ImGuiExample)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(/usr/include/imgui)
# Dear ImGui를 가져오기 (submodule 또는 다운로드 사용)
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

add_executable(imgui_example main.cpp)

target_link_libraries(imgui_example PRIVATE
  glfw
  OpenGL::GL
  imgui
  imgui_impl_glfw
  imgui_impl_opengl3
)

target_compile_features(imgui_example PRIVATE cxx_std_11)


