#include <imgui/imgui.h>
#include <GLFW/glfw3.h> // OpenGL 백엔드 예시
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <stdio.h>

// GLFW 및 ImGui 초기화 (간단히)
static void init_imgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// Winamp 스타일 설정
void setup_winamp_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 8.0f; // 둥근 버튼
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // 어두운 배경
    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.8f, 0.8f, 1.0f); // 네온 청록색 버튼
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.6f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // 메탈릭 느낌
}

int main(int, char**) {
    // GLFW 초기화
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(500, 400, "Pump Control Panel", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ImGui 초기화
    init_imgui(window);
    setup_winamp_style();

    // 상태 변수
    bool pump_on = false;
    float flow_rate = 5.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 창 생성
        ImGui::Begin("Pump Control", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextColored(pump_on ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                           "Status: %s", pump_on ? "ON" : "OFF");

        // 시작/정지 버튼
        if (ImGui::Button("Start", ImVec2(100, 40))) {
            pump_on = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop", ImVec2(100, 40))) {
            pump_on = false;
        }

        // 유량 슬라이더
        ImGui::SliderFloat("Flow Rate (L/min)", &flow_rate, 0.0f, 10.0f, "%.1f");

        // 경고등
        static bool warning = false;
        ImGui::Checkbox("Warning", &warning);
        if (warning) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "!");
        }

        ImGui::End();
        ImGui::Render();

        // 렌더링
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
