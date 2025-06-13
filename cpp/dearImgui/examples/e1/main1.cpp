#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

int main() {
    // GLFW 초기화
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui Example", NULL, NULL);
    
    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // ImGui 스타일 설정 (선택 사항)
    ImGui::StyleColorsDark();

    // ImGui + GLFW + OpenGL3 초기화
    ImGui_ImplGlfw_InitForOpenGL(window, true); // GLFW 초기화
    ImGui_ImplOpenGL3_Init("#version 330 core"); // OpenGL3 초기화

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // 새 프레임 시작
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // ImGui UI 생성
        ImGui::ShowDemoWindow(); // 데모 창 표시
        
        // 렌더링
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // 정리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}

// g++ main.cpp -I/usr/include/imgui -lglfw -lGL -lGLEW -limgui -o imgui_example
// g++ main.cpp -I/usr/include/imgui -I/usr/include/stb -lglfw -lGL -lGLEW -limgui -L/usr/lib/x86_64-linux-gnu -lstb -o imgui_example

