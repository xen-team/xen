#include "window.hpp"

#include <data/image.hpp>
#include <render/renderer.hpp>
#include <render/render_system.hpp>

// GLEW is needed for V-sync management
#include <GL/glew.h>
#if defined(XEN_IS_PLATFORM_WINDOWS)
#include "GL/wglew.h"
#elif defined(XEN_IS_PLATFORM_LINUX)
#include "GL/glxew.h"
#endif

#include <GLFW/glfw3.h>

#if !defined(XEN_NO_OVERLAY)
// Needed to set ImGui's callbacks
#include "imgui_impl_glfw.h"
#endif

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#if defined(XEN_IS_PLATFORM_EMSCRIPTEN)
#include <emscripten/html5.h>
#endif

namespace xen {
Window::Window(
    RenderSystem& render_system, Vector2ui const& size, std::string const& title, WindowSetting settings,
    uint8_t anti_aliasing_sample_count
) : render_system{&render_system}
{
    ZoneScopedN("Window::Window");

    Log::debug("[Window] Initializing...");

    glfwSetErrorCallback([](int error_code, char const* description) {
        Log::verror("[GLFW] {} (error code {}).", description, error_code);
    });

    if (!glfwInit()) {
        throw std::runtime_error("Error: Failed to initialize GLFW");
    }

#if !defined(USE_OPENGL_ES)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif

#if defined(XEN_CONFIG_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

#if defined(XEN_IS_PLATFORM_MAC) // Setting the OpenGL forward compatibility is required on macOS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif

    glfwWindowHint(GLFW_FOCUSED, static_cast<int>(settings & WindowSetting::FOCUSED));
    glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(settings & WindowSetting::RESIZABLE));
    glfwWindowHint(GLFW_VISIBLE, static_cast<int>(settings & WindowSetting::VISIBLE));
    glfwWindowHint(GLFW_DECORATED, static_cast<int>(settings & WindowSetting::DECORATED));
    glfwWindowHint(GLFW_AUTO_ICONIFY, static_cast<int>(settings & WindowSetting::AUTO_MINIMIZE));
    glfwWindowHint(GLFW_FLOATING, static_cast<int>(settings & WindowSetting::ALWAYS_ON_TOP));
    glfwWindowHint(GLFW_MAXIMIZED, static_cast<int>(settings & WindowSetting::MAXIMIZED));
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    glfwWindowHint(GLFW_CENTER_CURSOR, static_cast<int>(settings & WindowSetting::CENTER_CURSOR));
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, static_cast<int>(settings & WindowSetting::TRANSPARENT_FB));
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, static_cast<int>(settings & WindowSetting::AUTOFOCUS));
#endif

    glfwWindowHint(GLFW_SAMPLES, anti_aliasing_sample_count);

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    static constexpr std::array<std::pair<int, int>, 8> gl_versions = {
        {{4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0}, {3, 3}}
    };

    for (auto [major, minor] : gl_versions) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

        window_handle = glfwCreateWindow(
            static_cast<int>(size.x), static_cast<int>(size.y), title.c_str(), nullptr, glfwGetCurrentContext()
        );

        if (window_handle) {
            break;
        }

        if (glfwGetError(nullptr) == GLFW_VERSION_UNAVAILABLE) {
            Log::verror("[Window] OpenGL {}.{} unsupported; attempting to fallback to a lower version.", major, minor);
            continue;
        }

        close();

        throw std::runtime_error("Error: Failed to create GLFW Window");
    }
#else
    window_handle = glfwCreateWindow(
        static_cast<int>(size.x), static_cast<int>(size.y), title.c_str(), nullptr, glfwGetCurrentContext()
    );
#endif

    glfwSetWindowUserPointer(window_handle, this);

    int width, height;
    glfwGetWindowSize(window_handle, &width, &height);
    this->size = Vector2ui(width, height);

    glfwGetWindowPos(window_handle, &position.x, &position.y);

    if (glfwGetCurrentContext() == nullptr) {
        glfwMakeContextCurrent(window_handle);
    }

    Renderer::init();
    set_clear_color(Color(0.15f, 0.15f, 0.15f));

    glfwSetFramebufferSizeCallback(window_handle, [](GLFWwindow* window_handle, int new_width, int new_height) {
        static_cast<Window const*>(glfwGetWindowUserPointer(window_handle))
            ->render_system->resize_viewport(Vector2ui(new_width, new_height));
    });

#if !defined(XEN_NO_OVERLAY)
    Overlay::init(window_handle);
#endif

    ++ref_counter;

    Log::debug("[Window] Initialized");
}

void Window::set_clear_color(Color const& color) const
{
    Renderer::clear_color(color);
}

void Window::set_title(std::string const& title) const
{
    glfwSetWindowTitle(window_handle, title.c_str());
}

void Window::set_icon(Image const& image) const
{
    if (image.empty()) {
        Log::error("[Window] Empty image given as window icon.");
        return;
    }

    if (image.get_colorspace() != ImageColorspace::RGBA) {
        Log::error("[Window] The window icon can only be created from an image having an RGBA colorspace.");
        return;
    }

    if (image.get_data_type() != ImageDataType::BYTE) {
        Log::error("[Window] The window icon can only be created from an image having byte data.");
        return;
    }

    GLFWimage const icon = {
        static_cast<int>(image.get_width()), static_cast<int>(image.get_height()),
        const_cast<unsigned char*>(static_cast<uint8_t const*>(image.data()))
    };
    glfwSetWindowIcon(window_handle, 1, &icon);
}

void Window::resize(Vector2ui const& size)
{
    glfwSetWindowSize(window_handle, static_cast<int>(size.x), static_cast<int>(size.y));

    int width, height;
    glfwGetWindowSize(window_handle, &width, &height);
    this->size = Vector2ui(width, height);
}

void Window::make_fullscreen()
{
    int width, height;
    glfwGetWindowSize(window_handle, &width, &height);
    size = Vector2ui(width, height);

    glfwGetWindowPos(window_handle, &position.x, &position.y);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    GLFWvidmode const* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window_handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

void Window::make_windowed()
{
    glfwSetWindowMonitor(window_handle, nullptr, position.x, position.y, size.x, size.y, GLFW_DONT_CARE);
}

void Window::enable_face_culling(bool value) const
{
    if (value) {
        Renderer::enable(Capability::CULL);
    }
    else {
        Renderer::disable(Capability::CULL);
    }
}

bool Window::recover_vertical_sync_state() const
{
    ZoneScopedN("Window::recover_vertical_sync_state");

#if defined(XEN_IS_PLATFORM_WINDOWS)
    if (wglGetExtensionsStringEXT()) {
        return static_cast<bool>(wglGetSwapIntervalEXT());
    }

    return true;
#elif defined(XEN_IS_PLATFORM_LINUX)
    if (glXQueryExtensionsString(glXGetCurrentDisplay(), 0)) {
        uint interval{};
        glXQueryDrawable(glXGetCurrentDisplay(), glXGetCurrentDrawable(), GLX_SWAP_INTERVAL_EXT, &interval);

        return static_cast<bool>(interval);
    }

    return true;
#elif defined(XEN_IS_PLATFORM_MAC)
    return true;
#else
    Log::warning("Vertical synchronization unsupported.");
    return false;
#endif
}

void Window::enable_vertical_sync([[maybe_unused]] bool value) const
{
    ZoneScopedN("Window::enable_vertical_sync");

#if defined(XEN_IS_PLATFORM_WINDOWS)
    if (wglGetExtensionsStringEXT()) {
        wglSwapIntervalEXT(static_cast<int>(value));
        return;
    }
#elif defined(XEN_IS_PLATFORM_LINUX)
    if (glXQueryExtensionsString(glXGetCurrentDisplay(), 0)) {
        glXSwapIntervalEXT(glXGetCurrentDisplay(), glXGetCurrentDrawable(), value);
        glXSwapIntervalMESA(static_cast<uint>(value));
        return;
    }
#elif defined(XEN_IS_PLATFORM_MAC)
    glfwSwapInterval(value);
#else
    Log::warning("Vertical synchronization unsupported.");
#endif
}

void Window::set_cursor_state(Cursor::State state) const
{
    glfwSetInputMode(window_handle, GLFW_CURSOR, state);
}

void Window::add_key_callback(
    Keyboard::Key key, std::function<void(float)> action_press, Input::ActionTrigger frequency,
    std::function<void()> action_release
)
{
    std::get<0>(callbacks).emplace_back(key, std::move(action_press), frequency, std::move(action_release));
    update_callbacks();
}

void Window::add_mouse_button_callback(
    Mouse::Button button, std::function<void(float)> action_press, Input::ActionTrigger frequency,
    std::function<void()> action_release
)
{
    std::get<1>(callbacks).emplace_back(button, std::move(action_press), frequency, std::move(action_release));
    update_callbacks();
}

void Window::set_mouse_scroll_callback(std::function<void(double, double)> func)
{
    std::get<2>(callbacks) = std::move(func);
    update_callbacks();
}

void Window::set_mouse_move_callback(std::function<void(double, double)> func)
{
    std::get<3>(callbacks) = std::make_tuple(size.x / 2, size.y / 2, std::move(func));
    update_callbacks();
}

void Window::set_close_callback(std::function<void()> func)
{
    close_callback = std::move(func);

    glfwSetWindowCloseCallback(window_handle, [](GLFWwindow* window_handle) {
        static_cast<Window const*>(glfwGetWindowUserPointer(window_handle))->close_callback();
    });
}

void Window::update_callbacks() const
{
    ZoneScopedN("Window::update_callbacks");

#if !defined(XEN_NO_OVERLAY)
    // Monitor events
    glfwSetMonitorCallback([](GLFWmonitor* monitorHandle, int event) {
        ImGui_ImplGlfw_MonitorCallback(monitorHandle, event);
    });
#endif

#if !defined(XEN_NO_OVERLAY)
    // Window focus
    glfwSetWindowFocusCallback(window_handle, [](GLFWwindow* window_handle, int focused) {
        ImGui_ImplGlfw_WindowFocusCallback(window_handle, focused);
    });
#endif

    // Keyboard inputs
    if (!std::get<0>(callbacks).empty()) {
        glfwSetKeyCallback(window_handle, [](GLFWwindow* window_handle, int key, int scancode, int action, int mods) {
#if !defined(XEN_NO_OVERLAY)
            ImGui_ImplGlfw_KeyCallback(window_handle, key, scancode, action, mods);

            // Key callbacks should not be executed if the overlay requested keyboard focus
            if (ImGui::GetIO().WantCaptureKeyboard)
                return;
#endif

            InputCallbacks& callbacks = static_cast<Window*>(glfwGetWindowUserPointer(window_handle))->callbacks;
            auto const& keyCallbacks = std::get<0>(callbacks);

            for (auto const& callback : keyCallbacks) {
                if (key != std::get<0>(callback))
                    continue;

                auto& actions = std::get<InputActions>(callbacks);

                if (action == GLFW_PRESS) {
                    actions.emplace(key, std::make_pair(std::get<1>(callback), std::get<2>(callback)));
                }
                else if (action == GLFW_RELEASE) {
                    actions.erase(key);

                    auto const& action_release = std::get<3>(callback);

                    if (action_release)
                        action_release();
                }
            }
        });
    }

#if !defined(XEN_NO_OVERLAY)
    // Unicode character inputs
    glfwSetCharCallback(window_handle, [](GLFWwindow* window_handle, uint codePoint) {
        ImGui_ImplGlfw_CharCallback(window_handle, codePoint);
    });
#endif

#if !defined(XEN_NO_OVERLAY)
    // Cursor enter event
    glfwSetCursorEnterCallback(window_handle, [](GLFWwindow* window_handle, int entered) {
        ImGui_ImplGlfw_CursorEnterCallback(window_handle, entered);
    });
#endif

    // Mouse buttons inputs
    if (!std::get<1>(callbacks).empty()) {
        glfwSetMouseButtonCallback(window_handle, [](GLFWwindow* window_handle, int button, int action, int mods) {
#if !defined(XEN_NO_OVERLAY)
            ImGui_ImplGlfw_MouseButtonCallback(window_handle, button, action, mods);

            // Mouse buttons callbacks should not be executed if the overlay requested mouse focus
            if (ImGui::GetIO().WantCaptureMouse)
                return;
#endif

            InputCallbacks& callbacks = static_cast<Window*>(glfwGetWindowUserPointer(window_handle))->callbacks;
            auto const& mouseCallbacks = std::get<1>(callbacks);

            for (auto const& callback : mouseCallbacks) {
                if (button != std::get<0>(callback))
                    continue;

                auto& actions = std::get<InputActions>(callbacks);

                if (action == GLFW_PRESS) {
                    actions.emplace(button, std::make_pair(std::get<1>(callback), std::get<2>(callback)));
                }
                else if (action == GLFW_RELEASE) {
                    actions.erase(button);

                    auto const& action_release = std::get<3>(callback);

                    if (action_release)
                        action_release();
                }
            }
        });
    }

    // Mouse scroll input
    if (std::get<2>(callbacks)) {
        glfwSetScrollCallback(window_handle, [](GLFWwindow* window_handle, double xOffset, double yOffset) {
#if !defined(XEN_NO_OVERLAY)
            ImGui_ImplGlfw_ScrollCallback(window_handle, xOffset, yOffset);

            // Scroll callback should not be executed if the overlay requested mouse focus
            if (ImGui::GetIO().WantCaptureMouse)
                return;
#endif

            const auto& scrollCallback =
                std::get<2>(static_cast<Window*>(glfwGetWindowUserPointer(window_handle))->callbacks);
            scrollCallback(xOffset, yOffset);
        });
    }

    // Mouse move input
    if (std::get<2>(std::get<3>(callbacks))) {
        glfwSetCursorPosCallback(window_handle, [](GLFWwindow* window_handle, double xPosition, double yPosition) {
            MouseMoveCallback& moveCallback =
                std::get<3>(static_cast<Window*>(glfwGetWindowUserPointer(window_handle))->callbacks);

            double& xPrevPos = std::get<0>(moveCallback);
            double& yPrevPos = std::get<1>(moveCallback);

            std::get<2>(moveCallback)(xPosition - xPrevPos, yPosition - yPrevPos);

            xPrevPos = xPosition;
            yPrevPos = yPosition;
        });
    }
}

bool Window::run(float delta_time)
{
    ZoneScopedN("Window::run");

    if (glfwWindowShouldClose(window_handle))
        return false;

    process_inputs(delta_time);

#if !defined(XEN_NO_OVERLAY)
    if (overlay_enabled && !overlay.empty())
        overlay.render();
#endif

    {
        ZoneScopedN("glfwSwapBuffers");
        TracyGpuZone("SwapBuffers") glfwSwapBuffers(window_handle);
    }

#if defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    emscripten_webgl_commit_frame();
#endif

    {
        TracyGpuZone("TracyGpuCollect") TracyGpuCollect
    }

    return true;
}

Vector2f Window::recover_mouse_position() const
{
    double xPos{};
    double yPos{};
    glfwGetCursorPos(window_handle, &xPos, &yPos);

    return Vector2f(static_cast<float>(xPos), static_cast<float>(yPos));
}

void Window::process_inputs(float delta_time)
{
    ZoneScopedN("Window::process_inputs");

    {
        ZoneScopedN("glfwPollEvents");
        glfwPollEvents();
    }

    auto& actions = std::get<InputActions>(callbacks);
    auto actionIter = actions.cbegin();

    while (actionIter != actions.cend()) {
        auto const& action = actionIter->second;

        // An action consists of two parts:
        // - A callback associated to the triggered key or button
        // - A value indicating if it should be executed only once or every frame

        action.first(delta_time);

        // Removing the current action if ONCE is given, or simply increment the iterator
        if (action.second == Input::ONCE)
            actionIter =
                actions.erase(actionIter); // std::unordered_map::erase(iter) returns an iterator on the next element
        else
            ++actionIter;
    }
}

void Window::set_should_close() const
{
    glfwSetWindowShouldClose(window_handle, true);
}

void Window::close()
{
    ZoneScopedN("Window::close");

    if (!window_handle.is_valid())
        return;

    Log::debug("[Window] Closing...");

    --ref_counter;

    if (ref_counter == 0) {
#if !defined(XEN_NO_OVERLAY)
        Overlay::destroy();
#endif

        {
            ZoneScopedN("glfwTerminate");
            glfwTerminate();
        }
        window_handle = nullptr;
    }

    Log::debug("[Window] Closed");
}

}
