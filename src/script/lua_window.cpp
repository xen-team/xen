#include <data/image.hpp>
#include <render/render_system.hpp>
#include <render/window.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_window_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Window> window = state.new_usertype<Window>(
            "Window",
            sol::constructors<
                Window(RenderSystem&, Vector2ui const&), Window(RenderSystem&, Vector2ui const&, std::string const&),
                Window(RenderSystem&, Vector2ui const&, std::string const&, WindowSetting),
                Window(RenderSystem&, Vector2ui const&, std::string const&, WindowSetting, uint8_t)>()
        );
        window["get_size"] = &Window::get_size;
        window["get_width"] = &Window::get_width;
        window["get_height"] = &Window::get_height;
#if !defined(XEN_NO_OVERLAY)
        window["get_overlay"] = &Window::get_overlay;
#endif
        window["set_clear_color"] = sol::overload(
            [](const Window& w, const Color& c) { w.set_clear_color(c); },
            PickOverload<const Color&>(&Window::set_clear_color)
        );
        window["set_title"] = &Window::set_title;
        window["set_icon"] = &Window::set_icon;
        window["resize"] = &Window::resize;
        window["make_fullscreen"] = &Window::make_fullscreen;
        window["make_windowed"] = &Window::make_windowed;
        window["enable_face_culling"] = sol::overload(
            [](Window const& w) { w.enable_face_culling(); }, PickOverload<bool>(&Window::enable_face_culling)
        );
        window["disable_face_culling"] = &Window::disable_face_culling;
        window["recover_vertical_sync_state"] = &Window::recover_vertical_sync_state;
        window["enable_vertical_sync"] = sol::overload(
            [](Window const& w) { w.enable_vertical_sync(); }, PickOverload<bool>(&Window::enable_vertical_sync)
        );
        window["disable_vertical_sync"] = &Window::disable_vertical_sync;
        window["show_cursor"] = &Window::show_cursor;
        window["hide_cursor"] = &Window::hide_cursor;
        window["disable_cursor"] = &Window::disable_cursor;
        window["add_key_callback"] = sol::overload(
            [](Window& w, Keyboard::Key k, std::function<void(float)> c) { w.add_key_callback(k, std::move(c)); },
            [](Window& w, Keyboard::Key k, std::function<void(float)> c, Input::ActionTrigger f) {
                w.add_key_callback(k, std::move(c), f);
            },
            PickOverload<Keyboard::Key, std::function<void(float)>, Input::ActionTrigger, std::function<void()>>(
                &Window::add_key_callback
            )
        );
        window["add_mouse_button_callback"] = sol::overload(
            [](Window& w, Mouse::Button b, std::function<void(float)> c) {
                w.add_mouse_button_callback(b, std::move(c));
            },
            [](Window& w, Mouse::Button b, std::function<void(float)> c, Input::ActionTrigger f) {
                w.add_mouse_button_callback(b, std::move(c), f);
            },
            PickOverload<Mouse::Button, std::function<void(float)>, Input::ActionTrigger, std::function<void()>>(
                &Window::add_mouse_button_callback
            )
        );
        window["set_mouse_scroll_callback"] = &Window::set_mouse_scroll_callback;
        window["set_mouse_move_callback"] = &Window::set_mouse_move_callback;
        window["set_close_callback"] = &Window::set_close_callback;
        window["update_callbacks"] = &Window::update_callbacks;
#if !defined(XEN_NO_OVERLAY)
        window["enable_overlay"] =
            sol::overload([](Window& w) { w.enable_overlay(); }, PickOverload<bool>(&Window::enable_overlay));
        window["disable_overlay"] = &Window::disable_overlay;
#endif
        window["run"] = &Window::run;
        window["recover_mouse_position"] = &Window::recover_mouse_position;

        state.new_enum<WindowSetting>(
            "WindowSetting", {{"FOCUSED", WindowSetting::FOCUSED},
                              {"RESIZABLE", WindowSetting::RESIZABLE},
                              {"VISIBLE", WindowSetting::VISIBLE},
                              {"DECORATED", WindowSetting::DECORATED},
                              {"AUTO_MINIMIZE", WindowSetting::AUTO_MINIMIZE},
                              {"ALWAYS_ON_TOP", WindowSetting::ALWAYS_ON_TOP},
                              {"MAXIMIZED", WindowSetting::MAXIMIZED},
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
                              {"CENTER_CURSOR", WindowSetting::CENTER_CURSOR},
                              {"TRANSPARENT_FB", WindowSetting::TRANSPARENT_FB},
                              {"AUTOFOCUS", WindowSetting::AUTOFOCUS},
#endif
                              {"DEFAULT", WindowSetting::DEFAULT},
                              {"NON_RESIZABLE", WindowSetting::NON_RESIZABLE},
                              {"WINDOWED", WindowSetting::WINDOWED},
                              {"BORDERLESS", WindowSetting::BORDERLESS},
                              {"INVISIBLE", WindowSetting::INVISIBLE}}
        );

        state.new_enum<Input::ActionTrigger>("Input", {{"ONCE", Input::ONCE}, {"ALWAYS", Input::ALWAYS}});

        state.new_enum<Keyboard::Key>(
            "Keyboard", {{"A", Keyboard::A},
                         {"B", Keyboard::B},
                         {"C", Keyboard::C},
                         {"D", Keyboard::D},
                         {"E", Keyboard::E},
                         {"F", Keyboard::F},
                         {"G", Keyboard::G},
                         {"H", Keyboard::H},
                         {"I", Keyboard::I},
                         {"J", Keyboard::J},
                         {"K", Keyboard::K},
                         {"L", Keyboard::L},
                         {"M", Keyboard::M},
                         {"N", Keyboard::N},
                         {"O", Keyboard::O},
                         {"P", Keyboard::P},
                         {"Q", Keyboard::Q},
                         {"R", Keyboard::R},
                         {"S", Keyboard::S},
                         {"T", Keyboard::T},
                         {"U", Keyboard::U},
                         {"V", Keyboard::V},
                         {"W", Keyboard::W},
                         {"X", Keyboard::X},
                         {"Y", Keyboard::Y},
                         {"Z", Keyboard::Z},

                         {"F1", Keyboard::F1},
                         {"F2", Keyboard::F2},
                         {"F3", Keyboard::F3},
                         {"F4", Keyboard::F4},
                         {"F5", Keyboard::F5},
                         {"F6", Keyboard::F6},
                         {"F7", Keyboard::F7},
                         {"F8", Keyboard::F8},
                         {"F9", Keyboard::F9},
                         {"F10", Keyboard::F10},
                         {"F11", Keyboard::F11},
                         {"F12", Keyboard::F12},

                         {"UP", Keyboard::UP},
                         {"DOWN", Keyboard::DOWN},
                         {"RIGHT", Keyboard::RIGHT},
                         {"LEFT", Keyboard::LEFT},

                         {"NUMLOCK", Keyboard::NUMLOCK},
                         {"NUM0", Keyboard::NUM0},
                         {"NUM1", Keyboard::NUM1},
                         {"NUM2", Keyboard::NUM2},
                         {"NUM3", Keyboard::NUM3},
                         {"NUM4", Keyboard::NUM4},
                         {"NUM5", Keyboard::NUM5},
                         {"NUM6", Keyboard::NUM6},
                         {"NUM7", Keyboard::NUM7},
                         {"NUM8", Keyboard::NUM8},
                         {"NUM9", Keyboard::NUM9},
                         {"DECIMAL", Keyboard::DECIMAL},
                         {"DIVIDE", Keyboard::DIVIDE},
                         {"MULTIPLY", Keyboard::MULTIPLY},
                         {"SUBSTRACT", Keyboard::SUBSTRACT},
                         {"ADD", Keyboard::ADD},

                         {"LEFT_SHIFT", Keyboard::LEFT_SHIFT},
                         {"RIGHT_SHIFT", Keyboard::RIGHT_SHIFT},
                         {"LEFT_CTRL", Keyboard::LEFT_CTRL},
                         {"RIGHT_CTRL", Keyboard::RIGHT_CTRL},
                         {"LEFT_ALT", Keyboard::LEFT_ALT},
                         {"RIGHT_ALT", Keyboard::RIGHT_ALT},

                         {"HOME", Keyboard::HOME},
                         {"END", Keyboard::END},
                         {"PAGEUP", Keyboard::PAGEUP},
                         {"PAGEDOWN", Keyboard::PAGEDOWN},
                         {"CAPSLOCK", Keyboard::CAPSLOCK},
                         {"SPACE", Keyboard::SPACE},
                         {"BACKSPACE", Keyboard::BACKSPACE},
                         {"INSERT", Keyboard::INSERT},
                         {"ESCAPE", Keyboard::ESCAPE},
                         {"PRINT_SCREEN", Keyboard::PRINT_SCREEN},
                         {"PAUSE", Keyboard::PAUSE}}
        );

        state.new_enum<Mouse::Button>(
            "Mouse", {{"LEFT_CLICK", Mouse::LEFT_CLICK},
                      {"RIGHT_CLICK", Mouse::RIGHT_CLICK},
                      {"MIDDLE_CLICK", Mouse::MIDDLE_CLICK}}
        );
    }
}
}