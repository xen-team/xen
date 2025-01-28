#include "window.hpp"

#include "debug/log.hpp"
#include "engine/engine.hpp"
#include "units/math.hpp"
#include "windows.hpp"

#include <GLFW/glfw3.h>
#include <magic_enum.hpp>

namespace xen {
static_assert(GLFW_KEY_LAST == static_cast<int16_t>(Key::Menu), "GLFW keys count does not match our keys enum count.");
static_assert(
    GLFW_MOUSE_BUTTON_LAST == static_cast<int16_t>(MouseButton::_8),
    "GLFW mouse button count does not match our mouse button enum count."
);

void callback_window_pos(GLFWwindow* glfw_window, int32_t x, int32_t y)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    if (window->fullscreen) {
        return;
    }

    window->pos = Vector2ui(x, y);
    window->on_pos(window->pos);
}

void callback_window_size(GLFWwindow* glfw_window, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

    if (window->fullscreen) {
        window->fullscreen_size = Vector2ui(width, height);
        window->on_size(window->fullscreen_size);
    }
    else {
        window->size = Vector2ui(width, height);
        window->on_size(window->size);
    }
}

void callback_window_close(GLFWwindow* glfw_window)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->closed = false;
    window->on_close();
}

void callback_window_focus(GLFWwindow* glfw_window, int32_t focused)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->focused = focused;
    window->on_focus(focused == GLFW_TRUE);

    Windows::get()->set_focused_window(window->id);
}

void callback_window_iconify(GLFWwindow* glfw_window, int32_t iconified)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->iconified = iconified;
    window->on_iconify(iconified);
}

void callback_framebuffer_size(GLFWwindow* glfw_window, int32_t width, int32_t height)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    if (window->fullscreen) {
        window->fullscreen_size = Vector2ui(width, height);
    }
    else {
        window->size = Vector2ui(width, height);
    }
    // TODO: only set this window to resized
    // Graphics::Get()->SetFramebufferResized(window->id);
}

void callback_cursor_enter(GLFWwindow* glfw_window, int32_t entered)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->window_selected = entered == GLFW_TRUE;
    window->on_enter(entered == GLFW_TRUE);
}

void callback_drop(GLFWwindow* glfw_window, int32_t count, char const** paths)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    std::vector<std::string> files(count);
    for (int32_t i = 0; i < count; i++) {
        files[i] = paths[i];
    }
    window->on_drop(files);
}

void callback_key(GLFWwindow* glfw_window, int32_t key, [[maybe_unused]] int32_t scancode, int32_t action, int32_t mods)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->on_key(static_cast<Key>(key), static_cast<InputAction>(action), InputMods(mods));
}

void callback_char(GLFWwindow* glfw_window, uint32_t codepoint)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->on_char(static_cast<char>(codepoint));
}

void callback_mouse_button(GLFWwindow* glfw_window, int32_t button, int32_t action, int32_t mods)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->on_mouse_button(static_cast<MouseButton>(button), static_cast<InputAction>(action), InputMods(mods));
}

void callback_cursor_pos(GLFWwindow* glfw_window, double x, double y)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->mouse_pos = Vector2d(x, y);
    window->on_mouse_pos(window->mouse_pos);
}

void callback_scroll(GLFWwindow* glfw_window, double x, double y)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->mouse_scroll = Vector2d(x, y);
    window->on_mouse_scroll(window->mouse_scroll);
}

double Window::smooth_scroll_wheel(double value, double delta)
{
    if (value != 0.0) {
        value -= delta * std::copysign(3.0, value);
        value = Math::deadband(0.08, value);
        return value;
    }

    return 0.0;
}

Window::Window(WindowId id) : id{id}, size{default_size}, title{default_name}, resizable{true}, focused{true}
{
    window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("GLFW failed to create the window");
    }

    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);

    glfwSetWindowAttrib(window, GLFW_DECORATED, static_cast<int>(!borderless));
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, static_cast<int>(resizable));
#ifndef PLATFORM_WAYLAND
    glfwSetWindowAttrib(window, GLFW_FLOATING, floating);
    pos.x = (get_current_monitor()->get_video_mode().width - size.x) / 2;
    pos.y = (get_current_monitor()->get_video_mode().height - size.y) / 2;
    glfwSetWindowPos(window, pos.x, pos.y);
#endif

    if (fullscreen) {
        set_fullscreen(true);
    }

    // Shows the glfw window.
    glfwShowWindow(window);

    // Sets the displays callbacks.
    glfwSetWindowPosCallback(window, callback_window_pos);
    glfwSetWindowSizeCallback(window, callback_window_size);
    glfwSetWindowCloseCallback(window, callback_window_close);
    glfwSetWindowFocusCallback(window, callback_window_focus);
    glfwSetWindowIconifyCallback(window, callback_window_iconify);
    glfwSetFramebufferSizeCallback(window, callback_framebuffer_size);
    glfwSetCursorEnterCallback(window, callback_cursor_enter);
    glfwSetDropCallback(window, callback_drop);
    glfwSetKeyCallback(window, callback_key);
    glfwSetCharCallback(window, callback_char);
    glfwSetMouseButtonCallback(window, callback_mouse_button);
    glfwSetCursorPosCallback(window, callback_cursor_pos);
    glfwSetScrollCallback(window, callback_scroll);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    closed = true;
}

void Window::update()
{
    float const delta = Engine::get()->get_delta().as_seconds();

    // Updates the position delta.
    mouse_pos_delta = delta * (mouse_last_pos - mouse_pos);
    mouse_last_pos = mouse_pos;

    // Updates the scroll delta.
    mouse_scroll_delta = delta * (mouse_last_scroll - mouse_scroll);
    mouse_last_scroll = mouse_scroll;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Window::set_size(Vector2ui const& size)
{
    this->size = size;
    glfwSetWindowSize(window, size.x, size.y);
}

void Window::set_pos([[maybe_unused]] Vector2ui const& pos)
{
#ifndef PLATFORM_WAYLAND
    this->pos = pos;
    glfwSetWindowPos(window, pos.x, pos.y);
#else
    Log::warning("Failed to set window pos under Wayland");
#endif
}

void Window::set_title(std::string_view title)
{
    this->title = title;
    glfwSetWindowTitle(window, title.data());
}

void Window::set_icons([[maybe_unused]] std::vector<std::filesystem::path> const& filenames)
{
    // std::vector<std::unique_ptr<Bitmap>> bitmaps;
    // std::vector<GLFWimage> icons;

    // for (const auto &filename : filenames) {
    // 	auto bitmap = std::make_unique<Bitmap>(filename);

    // 	if (!bitmap) continue;

    // 	GLFWimage icon = {};
    // 	icon.width = bitmap->GetSize().x;
    // 	icon.height = bitmap->GetSize().y;
    // 	icon.pixels = bitmap->GetData().get();
    // 	icons.emplace_back(icon);
    // 	bitmaps.emplace_back(std::move(bitmap));
    // }

    // glfwSetWindowIcon(window, static_cast<int32_t>(icons.size()), icons.data());
}

void Window::set_borderless(bool borderless)
{
    this->borderless = borderless;
    glfwSetWindowAttrib(window, GLFW_DECORATED, !borderless);
}

void Window::set_resizable(bool resizable)
{
    this->resizable = resizable;
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable);
}

void Window::set_floating([[maybe_unused]] bool floating)
{
#ifndef PLATFORM_WAYLAND
    this->floating = floating;
    glfwSetWindowAttrib(window, GLFW_FLOATING, floating);
#else
    Log::warning("Failed to set window floating under Wayland");
#endif
}

void Window::set_fullscreen(bool fullscreen, Monitor* monitor)
{
    Monitor const* selected = monitor ? monitor : get_current_monitor();
    GLFWvidmode const video_mode = selected->get_video_mode();

    this->fullscreen = fullscreen;

    if (fullscreen) {
#ifdef ACID_DEBUG
        Log::Out("Window is going fullscreen\n");
#endif
        glfwSetWindowMonitor(
            window, selected->get_monitor(), 0, 0, video_mode.width, video_mode.height, GLFW_DONT_CARE
        );
    }
    else {
#ifdef ACID_DEBUG
        Log::Out("Window is going windowed\n");
#endif
        pos = ((Vector2i(video_mode.width, video_mode.height) - size) / 2) + selected->get_pos();
        glfwSetWindowMonitor(window, nullptr, pos.x, pos.y, size.x, size.y, GLFW_DONT_CARE);
    }
}

void Window::set_iconified(bool iconify)
{
    if (!iconified && iconify) {
        glfwIconifyWindow(window);
    }
    else if (iconified && !iconify) {
        glfwRestoreWindow(window);
    }
}

std::string Window::get_clipboard() const
{
    return glfwGetClipboardString(window);
}

void Window::set_clipboard(std::string const& str) const
{
    glfwSetClipboardString(window, str.c_str());
}

void Window::set_cursor_hidden(bool hidden)
{
    if (cursor_hidden == hidden) {
        return;
    }

    glfwSetInputMode(window, GLFW_CURSOR, hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    if (!hidden && cursor_hidden) {
        set_mouse_pos(mouse_pos);
    }

    cursor_hidden = hidden;
}

// void Window::SetCursor(const Cursor *cursor) {
// 	glfwSetCursor(window, cursor ? cursor->cursor : nullptr);
// }

InputAction Window::get_key(Key key) const
{
    auto const state = glfwGetKey(window, static_cast<int32_t>(key));
    return static_cast<InputAction>(state);
}

InputAction Window::get_mouse_button(MouseButton button) const
{
    auto const state = glfwGetMouseButton(window, static_cast<int8_t>(button));
    return static_cast<InputAction>(state);
}

void Window::set_mouse_pos(Vector2d const& mouse_pos)
{
    this->mouse_last_pos = mouse_pos;
    this->mouse_pos = mouse_pos;
    glfwSetCursorPos(window, mouse_pos.x, mouse_pos.y);
}

void Window::set_mouse_scroll(Vector2d const& mouse_scroll)
{
    this->mouse_last_scroll = mouse_scroll;
    this->mouse_scroll = mouse_scroll;
}

std::string_view Window::key_as_str(Key key)
{
    return magic_enum::enum_name(key);
}

static constexpr int32_t OverlappingArea(Vector2i const& l1, Vector2i const& r1, Vector2i const& l2, Vector2i const& r2)
{
    int const x1 = std::max(l1.x, l2.x);
    int const y1 = std::max(l1.y, l2.y);
    int const x2 = std::min(r1.x, r2.x);
    int const y2 = std::min(r1.y, r2.y);

    if (x2 <= x1 || y2 <= y1) {
        return 0;
    }

    return (x2 - x1) * (y2 - y1);
}

// *
Monitor const* Window::get_current_monitor() const
{
    if (fullscreen) {
        GLFWmonitor const* window_monitor = glfwGetWindowMonitor(window);
        for (auto const& monitor : Windows::get()->get_monitors()) {
            if (monitor->monitor != window_monitor) {
                continue;
            }

            return monitor.get();
        }
        return nullptr;
    }
    std::multimap<int32_t, Monitor const*> ranked_monitor;
    auto where = ranked_monitor.end();

    for (auto const& monitor : Windows::get()->get_monitors()) {
        where = ranked_monitor.insert(
            where, {OverlappingArea(
                        monitor->get_workarea_pos(), monitor->get_workarea_pos() + monitor->get_workarea_size(), pos,
                        pos + size
                    ),
                    monitor.get()}
        );
    }

    if (ranked_monitor.begin()->first > 0) {
        return ranked_monitor.begin()->second;
    }

    return nullptr;
}
}