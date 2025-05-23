#pragma once

#include <data/owner_value.hpp>
#include <render/overlay.hpp>
#include <utils/enum_utils.hpp>
#include <utils/input.hpp>

namespace xen {
class Image;
class RenderSystem;
class Window;
using WindowPtr = std::unique_ptr<Window>;

using KeyboardCallbacks =
    std::vector<std::tuple<int, std::function<void(float)>, Input::ActionTrigger, std::function<void()>>>;
using MouseButtonCallbacks =
    std::vector<std::tuple<int, std::function<void(float)>, Input::ActionTrigger, std::function<void()>>>;
using MouseScrollCallback = std::function<void(double, double)>;
using MouseMoveCallback = std::tuple<double, double, std::function<void(double, double)>>;
using InputActions = std::unordered_map<int, std::pair<std::function<void(float)>, Input::ActionTrigger>>;
using InputCallbacks =
    std::tuple<KeyboardCallbacks, MouseButtonCallbacks, MouseScrollCallback, MouseMoveCallback, InputActions>;
using CloseCallback = std::function<void()>;

enum class WindowSetting : uint {
    FOCUSED = 1,   ///< Forces the window to take the focus.
    RESIZABLE = 2, ///< Makes the window able to be resized, either by dragging the edges & corners or by maximizing it.
    VISIBLE = 4,   ///< Makes the window visible.
    DECORATED = 8, ///< Defines if there are borders, minimize/maximize/close buttons, etc.
    AUTO_MINIMIZE = 16, ///< Automatically minimizes the full-screen window on focus loss.
    ALWAYS_ON_TOP = 32, ///< Forces the window to be floating, on top of everything on the screen.
    MAXIMIZED = 64,     ///< Maximizes the window, taking the whole screen space.
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    CENTER_CURSOR = 128,  ///< Centers the cursor on created full-screen windows.
    TRANSPARENT_FB = 256, ///< Blends what is behind the window according to the framebuffer's alpha channel.
    AUTOFOCUS = 512,      ///< Focuses the window every time it is shown.
#endif

    DEFAULT = FOCUSED | RESIZABLE | VISIBLE | DECORATED, ///< Default window settings.
    NON_RESIZABLE = FOCUSED | VISIBLE | DECORATED,       ///< Default window settings without resizing capabilities.
    WINDOWED = DEFAULT | MAXIMIZED,                      ///< Windowed full-screen window (with decorations).
    BORDERLESS = FOCUSED /*| MAXIMIZED*/ | VISIBLE,      ///< Borderless full-screen window (without decorations).
    INVISIBLE = 0                                        ///< Invisible window.
};
MAKE_ENUM_FLAG(WindowSetting)

/// Graphical window to render the scenes on, with input custom actions.
class Window {
    friend class RenderSystem;

public:
    /// Creates a window.
    /// \param render_system Render system containing this window.
    /// \param width Width of the window.
    /// \param height Height of the window.
    /// \param title Title of the window.
    /// \param settings Settings to create the window with.
    /// \param anti_aliasing_sample_count Number of anti-aliasing samples.
    /// \note The width & height are to be considered just hints; the window manager remains responsible for the actual
    /// dimensions, which may be lower.
    ///   This can notably happen when the requested window size exceeds what the screens can display. The actual
    ///   window's size can be queried afterward.
    /// \see get_width(), get_height()
    Window(
        RenderSystem& render_system, Vector2ui const& size, std::string const& title = {},
        WindowSetting settings = WindowSetting::DEFAULT, uint8_t anti_aliasing_sample_count = 1
    );
    Window(Window const&) = delete;
    Window(Window&&) noexcept = default;

    Window& operator=(Window const&) = delete;
    Window& operator=(Window&&) noexcept = default;

    ~Window() { close(); }

    Vector2ui get_size() const { return size; }

    uint get_width() const { return size.x; }

    uint get_height() const { return size.y; }

#if !defined(XEN_NO_OVERLAY)
    Overlay& get_overlay() { return overlay; }
#endif

    void set_clear_color(const Color& color) const;

    void set_title(std::string const& title) const;

    /// Sets an image as window icon.
    /// \param image Image to be set as the window's icon. Must have an RGBA colorspace & a byte data type.
    void set_icon(Image const& image) const;

    template <typename... Args>
    static WindowPtr create(Args&&... args)
    {
        return std::make_unique<Window>(std::forward<Args>(args)...);
    }

    /// Resizes the window.
    /// \param width New window width.
    /// \param height New window height.
    /// \note The width & height are to be considered just hints; the window manager remains responsible for the actual
    /// dimensions, which may be lower.
    ///   This can notably happen when the requested window size exceeds what the screens can display. The actual
    ///   window's size can be queried afterward.
    /// \see get_width(), get_height()
    void resize(Vector2ui const& size);

    /// Sets the window in a fullscreen mode, taking the whole main monitor's screen.
    /// \note To quit fullscreen, call make_windowed().
    /// \see make_windowed()
    void make_fullscreen();

    /// Sets the window in its windowed mode.
    void make_windowed();

    /// Changes the face culling's state.
    /// Enables or disables face culling according to the given parameter.
    /// \param value Value to apply.
    void enable_face_culling(bool value = true) const;

    /// Disables the face culling.
    void disable_face_culling() const { enable_face_culling(false); }

    /// Fetches the current vertical synchronization's state.
    /// \return True if vertical sync is enabled, false otherwise.
    bool recover_vertical_sync_state() const;

    /// Changes the vertical synchronization's state.
    /// Enables or disables vertical sync according to the given parameter.
    /// \param value Value to apply.
    void enable_vertical_sync(bool value = true) const;

    /// Disables vertical synchronization.
    void disable_vertical_sync() const { enable_vertical_sync(false); }

    /// Changes the cursor's state.
    /// Defines the new behavior of the mouse's cursor, if it should be shown, hidden or disabled.
    /// The functions show_cursor(), hide_cursor() & disable_cursor() can be used instead.
    /// \param state State to apply.
    void set_cursor_state(Cursor::State state) const;

    /// Shows the mouse cursor.
    /// Default behavior.
    void show_cursor() const { set_cursor_state(Cursor::State::NORMAL); }

    /// Hides the mouse cursor.
    /// The cursor becomes invisible while being inside the window's frame. It can go out of the window.
    void hide_cursor() const { set_cursor_state(Cursor::State::HIDDEN); }

    /// Disables the mouse cursor.
    /// The cursor always goes back to the window's center and becomes totally invisible. It can't go out of the window.
    void disable_cursor() const { set_cursor_state(Cursor::State::DISABLED); }

    /// Adds action(s) to be executed on keyboard's key press and/or release.
    /// \param key Key triggering the given action(s).
    /// \param action_press Action to be executed when the given key is pressed.
    /// \param frequency Frequency at which to execute the actions.
    /// \param action_release Action to be executed when the given key is released.
    void add_key_callback(
        Keyboard::Key key, std::function<void(float)> action_press, Input::ActionTrigger frequency = Input::ALWAYS,
        std::function<void()> action_release = nullptr
    );

    /// Adds action(s) to be executed on mouse button click and/or release.
    /// \param button Button triggering the given action(s).
    /// \param action_press Action to be executed when the given mouse button is pressed.
    /// \param frequency Frequency at which to execute the actions.
    /// \param action_release Action to be executed when the given mouse button is released.
    void add_mouse_button_callback(
        Mouse::Button button, std::function<void(float)> action_press, Input::ActionTrigger frequency = Input::ALWAYS,
        std::function<void()> action_release = nullptr
    );

    /// Sets the action to be executed on mouse wheel scroll.
    /// \param func Action to be executed when scrolling.
    void set_mouse_scroll_callback(std::function<void(double, double)> func);

    /// Sets the action to be executed on mouse move.
    /// \param func Action to be executed when the mouse is moved.
    void set_mouse_move_callback(std::function<void(double, double)> func);

    /// Sets the action to be executed on window close.
    /// \param func Action to be executed when the window is closed.
    void set_close_callback(std::function<void()> func);

    /// Associates all of the callbacks, making them active.
    void update_callbacks() const;

#if !defined(XEN_NO_OVERLAY)
    /// Changes the overlay's enabled state.
    /// \param enable True if it should be enabled, false otherwise.
    void enable_overlay(bool enable = true) { overlay_enabled = enable; }

    /// Disables the overlay.
    void disable_overlay() { enable_overlay(false); }
#endif

    /// Runs the window, refreshing its state by displaying the rendered scene, drawing the overlay, etc.
    /// \param delta_time Amount of time elapsed since the last frame.
    /// \return True if the window hasn't been required to close, false otherwise.
    bool run(float delta_time);

    /// Fetches the mouse position onto the window.
    /// \return 2D vector representing the mouse's position relative to the window.
    Vector2f recover_mouse_position() const;

private:
    static inline int ref_counter = 0;

    OwnerValue<GLFWwindow*, nullptr> window_handle{};
    RenderSystem* render_system{};

    Vector2ui size{};
    Vector2i position{};

    InputCallbacks callbacks{};
    CloseCallback close_callback{};

#if !defined(XEN_NO_OVERLAY)
    Overlay overlay{};
    bool overlay_enabled = true;
#endif

private:
    /// Processes actions corresponding to keyboard & mouse inputs.
    /// \param delta_time Amount of time elapsed since the last frame.
    void process_inputs(float delta_time);

    /// Tells the window that it should close.
    void set_should_close() const;

    /// Closes the window.
    void close();
};
}