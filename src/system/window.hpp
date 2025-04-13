#pragma once

#include "monitor.hpp"
#include "math/vector2.hpp"

#include <filesystem>
#include <rocket.hpp>

struct GLFWwindow;

namespace xen {
enum class Key : int16_t {
    Unknown = -1,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    _0 = 48,
    _1 = 49,
    _2 = 50,
    _3 = 51,
    _4 = 52,
    _5 = 53,
    _6 = 54,
    _7 = 55,
    _8 = 56,
    _9 = 57,
    Semicolon = 59,
    Equal = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    World1 = 161,
    World2 = 162,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    Numpad0 = 320,
    Numpad1 = 321,
    Numpad2 = 322,
    Numpad3 = 323,
    Numpad4 = 324,
    Numpad5 = 325,
    Numpad6 = 326,
    Numpad7 = 327,
    Numpad8 = 328,
    Numpad9 = 329,
    NumpadDecimal = 330,
    NumpadDivide = 331,
    NumpadMultiply = 332,
    NumpadSubtract = 333,
    NumpadAdd = 334,
    NumpadEnter = 335,
    NumpadEqual = 336,
    ShiftLeft = 340,
    ControlLeft = 341,
    AltLeft = 342,
    SuperLeft = 343,
    ShiftRight = 344,
    ControlRight = 345,
    AltRight = 346,
    SuperRight = 347,
    Menu = 348
};

enum class MouseButton : uint8_t {
    _1 = 0,
    _2 = 1,
    _3 = 2,
    _4 = 3,
    _5 = 4,
    _6 = 5,
    _7 = 6,
    _8 = 7,
    Left = _1,
    Right = _2,
    Middle = _3
};

enum class InputAction : uint8_t { Release = 0, Press = 1, Repeat = 2 };

enum InputMod : uint8_t {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3,
    CapsLock = 1 << 4,
    NumLock = 1 << 5
};
using InputMods = uint8_t;

using WindowId = size_t;

class XEN_API Window {
private:
    static constexpr Vector2ui default_size{1080, 720};
    static constexpr std::string default_name{"Xen App"};

    GLFWwindow* window = nullptr;
    WindowId id;

    Vector2ui size;
    Vector2ui fullscreen_size;
    Vector2ui pos;

    Vector2d mouse_last_pos;
    Vector2d mouse_pos;
    Vector2d mouse_pos_delta;

    Vector2d mouse_last_scroll;
    Vector2d mouse_scroll;
    Vector2d mouse_scroll_delta;

    std::string title;

    bool borderless = false;
    bool resizable = false;
    bool floating = false;
    bool fullscreen = false;

    bool closed = false;
    bool focused = false;
    bool iconified = false;

    bool cursor_hidden = false;
    bool window_selected = false;

public:
    rocket::signal<void(Vector2ui)> on_size;
    rocket::signal<void(Vector2ui)> on_pos;
    rocket::signal<void(std::string)> on_title;
    rocket::signal<void(bool)> on_borderless;
    rocket::signal<void(bool)> on_resizable;
    rocket::signal<void(bool)> on_floating;
    rocket::signal<void(bool)> on_fullscreen;
    rocket::signal<void()> on_close;
    rocket::signal<void(bool)> on_focus;
    rocket::signal<void(bool)> on_iconify;
    rocket::signal<void(bool)> on_enter;
    rocket::signal<void(std::vector<std::string>)> on_drop;
    rocket::signal<void(Key, InputAction, InputMods)> on_key;
    rocket::signal<void(char)> on_char;
    rocket::signal<void(MouseButton, InputAction, InputMods)> on_mouse_button;
    rocket::signal<void(Vector2d)> on_mouse_pos;
    rocket::signal<void(Vector2d)> on_mouse_scroll;

public:
    Window(WindowId id);
    ~Window();

    void update();

    static void bind();

    static void clear_all();

    static void clear_color();

    static void clear_depth();

    static void clear_stencil();

    [[nodiscard]] Vector2ui const& get_size() const { return size; }

    [[nodiscard]] uint16_t get_width() const { return size.x; }

    [[nodiscard]] uint16_t get_height() const { return size.y; }

    void set_size(Vector2ui const& size);

    [[nodiscard]] float get_aspect_ratio() const
    {
        return static_cast<float>(get_width()) / static_cast<float>(get_height());
    }

    [[nodiscard]] Vector2ui const& get_pos() const { return pos; }

    void set_pos(Vector2ui const& pos);

    [[nodiscard]] std::string_view get_title() const { return title; }

    void set_title(std::string_view title);

    void set_icons(std::vector<std::filesystem::path> const& filenames);

    [[nodiscard]] GLFWwindow* get_window() const { return window; }

    [[nodiscard]] bool is_borderless() const { return borderless; }

    void set_borderless(bool borderless);

    [[nodiscard]] bool is_resizable() const { return resizable; }

    void set_resizable(bool resizable);

    [[nodiscard]] bool is_floating() const { return floating; }

    void set_floating(bool floating);

    [[nodiscard]] bool is_fullscreen() const { return fullscreen; }

    void set_fullscreen(bool fullscreen, Monitor* monitor = nullptr);

    [[nodiscard]] bool is_closed() const { return closed; }

    [[nodiscard]] bool is_focused() const { return focused; }

    [[nodiscard]] bool is_iconified() const { return iconified; }

    void set_iconified(bool iconify);

    [[nodiscard]] std::string get_clipboard() const;

    void set_clipboard(std::string const& str) const;

    [[nodiscard]] bool is_window_selected() const { return window_selected; }

    [[nodiscard]] bool is_cursor_hidden() const { return cursor_hidden; }

    void set_cursor_hidden(bool hidden);

    // void set_cursor(const Cursor *cursor);

    [[nodiscard]] InputAction get_key(Key key) const;

    [[nodiscard]] InputAction get_mouse_button(MouseButton mouse_button) const;

    [[nodiscard]] Vector2d const& get_mouse_pos() const { return mouse_pos; }

    void set_mouse_pos(Vector2d const& mouse_pos);

    [[nodiscard]] Vector2d const& get_mouse_pos_delta() const { return mouse_pos_delta; }

    [[nodiscard]] Vector2d const& get_mouse_scroll() const { return mouse_scroll; }

    void set_mouse_scroll(Vector2d const& scroll);

    [[nodiscard]] Vector2d const& get_mouse_scroll_delta() const { return mouse_scroll_delta; }

    static std::string_view key_as_str(Key key);

    [[nodiscard]] Monitor const* get_current_monitor() const;

private:
    friend void callback_window_pos(GLFWwindow* glfw_window, int32_t x, int32_t y);
    friend void callback_window_size(GLFWwindow* glfw_window, int32_t width, int32_t height);
    friend void callback_window_close(GLFWwindow* glfw_window);
    friend void callback_window_focus(GLFWwindow* glfw_window, int32_t focused);
    friend void callback_window_iconify(GLFWwindow* glfw_window, int32_t iconified);
    friend void callback_framebuffer_size(GLFWwindow* glfw_window, int32_t width, int32_t height);
    friend void callback_cursor_enter(GLFWwindow* glfw_window, int32_t entered);
    friend void callback_drop(GLFWwindow* glfw_window, int32_t count, char const** paths);
    friend void callback_key(GLFWwindow* glfw_window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
    friend void callback_char(GLFWwindow* glfw_window, uint32_t codepoint);
    friend void callback_mouse_button(GLFWwindow* glfw_window, int32_t button, int32_t action, int32_t mods);
    friend void callback_cursor_pos(GLFWwindow* glfw_window, double xpos, double ypos);
    friend void callback_scroll(GLFWwindow* glfw_window, double xoffset, double yoffset);

    [[nodiscard]] static double smooth_scroll_wheel(double value, double delta);
};
}