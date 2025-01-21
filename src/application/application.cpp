/*
#include "app.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

struct sizez {
    uint16_t w;
    uint16_t h;
};

constexpr const char* window_name{"Brick Engine"};
constexpr sizez window_size{1980, 1080}; 
constexpr const char* project_name{"Simple Project"};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
// void processInput(GLFWwindow *window)
// {
//     if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, true);
    

//     if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//         camera.ProcessKeyboard(FORWARD, deltaTime);
//     if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//         camera.ProcessKeyboard(BACKWARD, deltaTime);
//     if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//         camera.ProcessKeyboard(LEFT, deltaTime);
//     if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//         camera.ProcessKeyboard(RIGHT, deltaTime);
// }

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

bool init_gl(GLFWwindow** window) {
    if (!glfwInit())
        return false;

    *window = glfwCreateWindow(window_size.w, window_size.h, window_name, nullptr, nullptr);

    if (!(*window))
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    return true;
}

void init_imgui(GLFWwindow** window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    ImGui_ImplGlfw_InitForOpenGL(*window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    auto& style = ImGui::GetStyle();
    style.DockingSeparatorSize = 6.f;
    // style.TabBarOverlineSize = 1.f;

    auto& colors = style.Colors;
    colors[ImGuiCol_WindowBg] = colors[ImGuiCol_WindowBg];
    colors[ImGuiCol_Border] = colors[ImGuiCol_WindowBg];
    colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_TitleBg];
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
    colors[ImGuiCol_TabSelected] = colors[ImGuiCol_TabDimmedSelected];
    colors[ImGuiCol_Tab] = colors[ImGuiCol_TabDimmedSelected];
    colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ImGuiCol_TabSelectedOverline];
}

bool init(GLFWwindow** window) {
    if (!init_gl(window))
        return false;

    init_imgui(window);

    return true;
}

app::app() {
    if (!init(&window))
        return;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    stbi_set_flip_vertically_on_load(true);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glfwSwapInterval(1);

    renderer = std::make_unique<render::RenderManager>();
    editor = std::make_unique<editor::EditorManager>();
}

void app::process() {
    renderer->get_default_scene().init_framebuffer(br_size{1.f, 1.f}, Pos2D{1.f, 1.f});

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // auto& default_scene = renderer->get_default_scene();

        // glViewport(0, 0, default_scene.get_framebuffer()->current_size.w, default_scene.get_framebuffer()->current_size.h);
        editor->draw();
        renderer->draw_debug(renderer->get_default_scene());

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

app::~app() {
    glfwTerminate();
}
*/

#include "application.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <utils/units.hpp>

#include <stb_image.h>
#include <filesystem>

constexpr const char* window_name{"Brick Engine"};
constexpr xsize window_size{1980, 1080}; 
constexpr const char* project_name{"Simple Project"};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
// void processInput(GLFWwindow *window)
// {
//     if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, true);
    

//     if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//         camera.ProcessKeyboard(FORWARD, deltaTime);
//     if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//         camera.ProcessKeyboard(BACKWARD, deltaTime);
//     if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//         camera.ProcessKeyboard(LEFT, deltaTime);
//     if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//         camera.ProcessKeyboard(RIGHT, deltaTime);
// }

void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

bool init_gl(GLFWwindow** window) {
    if (!glfwInit())
        return false;

    *window = glfwCreateWindow(window_size.x, window_size.y, window_name, nullptr, nullptr);

    if (!(*window))
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    return true;
}

void init_imgui(GLFWwindow** window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(*window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

bool init(GLFWwindow** window) {
    if (!init_gl(window))
        return false;

    init_imgui(window);

    return true;
}

app::app() {
    if (!init(&window))
        return;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    stbi_set_flip_vertically_on_load(true);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glfwSwapInterval(1);

    // renderer = &render::RenderManager::get_manager();
}

void app::process() {
    while (!glfwWindowShouldClose(window))
    {
        // editor->draw();
        // renderer->draw_debug();
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

app::~app() {
    glfwTerminate();
}