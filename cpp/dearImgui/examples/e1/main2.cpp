#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>

// 윈도우 관련 설정 (예시)
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
GLFWwindow* window;

// GLFW 에러 콜백
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

int main() {
    // GLFW 초기화
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    // OpenGL 버전 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 윈도우 생성
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ImGui Example", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // GLAD 초기화 (OpenGL 로딩 라이브러리)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // 폰트 설정 (선택 사항, 폰트 로드)
    // io.Fonts->AddFontDefault(); // 기본 폰트 사용
    // io.Fonts->AddFontFromFileTTF("path/to/your/font.ttf", 16.0f); // 사용자 폰트 로드

    // ImGui 스타일 설정 (선택 사항)
    ImGui::StyleColorsDark();

    // ImGui + GLFW + OpenGL3 초기화
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // 렌더링 루프
    while (!glfwWindowShouldClose(window)) {
        // 이벤트 처리
        glfwPollEvents();

        // ImGui 프레임 시작
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui UI 코드 (예시)
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some text.");
        ImGui::End();

        // 렌더링
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // ImGui 그리기

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

// g++ main.cpp -I/usr/include/imgui -I/usr/include/stb -lglfw -lGL -lGLEW -limgui -L/usr/lib/x86_64-linux-gnu -lstb -o imgui_example

