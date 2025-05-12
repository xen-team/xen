#include <render/overlay.hpp>
#include <render/texture.hpp>
#include <script/lua_wrapper.hpp>
#include <utility>
#include <utility>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_overlay_base_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Overlay> overlay = state.new_usertype<Overlay>("Overlay", sol::constructors<Overlay()>());
        overlay["empty"] = &Overlay::empty;
        overlay["add_window"] = sol::overload(
            [](Overlay& o, std::string t) { return &o.add_window(std::move(t)); },
            [](Overlay& o, std::string t, Vector2f const& s) { return &o.add_window(std::move(t), s); },
            PickOverload<std::string, Vector2f const&, Vector2f const&>(&Overlay::add_window)
        );
        overlay["has_keyboard_focus"] = &Overlay::has_keyboard_focus;
        overlay["has_mouse_focus"] = &Overlay::has_mouse_focus;
        overlay["render"] = &Overlay::render;
    }

    {
        sol::usertype<OverlayWindow> overlay_window = state.new_usertype<OverlayWindow>(
            "OverlayWindow", sol::constructors<
                                 OverlayWindow(std::string), OverlayWindow(std::string, Vector2f const&),
                                 OverlayWindow(std::string, Vector2f const&, Vector2f const&)>()
        );
        overlay_window["is_enabled"] = &OverlayWindow::is_enabled;
        overlay_window["enable"] =
            sol::overload([](OverlayWindow& r) { r.enable(); }, PickOverload<bool>(&OverlayWindow::enable));
        overlay_window["disable"] = &OverlayWindow::disable;
        overlay_window["add_label"] = &OverlayWindow::add_label;
        overlay_window["add_colored_label"] = &OverlayWindow::add_colored_label;
        overlay_window["add_button"] = &OverlayWindow::add_button;
        overlay_window["add_checkbox"] = &OverlayWindow::add_checkbox;
        overlay_window["add_slider"] = &OverlayWindow::add_slider;
        overlay_window["add_textbox"] = sol::overload(
            [](OverlayWindow& w, std::string l, std::function<void(std::string const&)> c) {
                return &w.add_textbox(std::move(l), std::move(c));
            },
            PickOverload<std::string, std::function<void(std::string const&)>, std::string>(&OverlayWindow::add_textbox)
        );
        overlay_window["add_text_area"] = sol::overload(
            [](OverlayWindow& w, std::string l, std::function<void(std::string const&)> c) {
                return &w.add_text_area(std::move(l), std::move(c));
            },
            [](OverlayWindow& w, std::string l, std::function<void(std::string const&)> c, std::string t) {
                return &w.add_text_area(std::move(l), std::move(c), std::move(t));
            },
            PickOverload<std::string, std::function<void(std::string const&)>, std::string, float>(
                &OverlayWindow::add_text_area
            )
        );
        overlay_window["add_list_box"] = sol::overload(
            [](OverlayWindow& w, std::string l, std::vector<std::string> e,
               std::function<void(std::string const&, size_t)> c) {
                return &w.add_list_box(std::move(l), std::move(e), std::move(c));
            },
            PickOverload<
                std::string, std::vector<std::string>, std::function<void(std::string const&, size_t)>, size_t>(
                &OverlayWindow::add_list_box
            )
        );
        overlay_window["add_dropdown"] = sol::overload(
            [](OverlayWindow& w, std::string l, std::vector<std::string> e,
               std::function<void(std::string const&, size_t)> c) {
                return &w.add_dropdown(std::move(l), std::move(e), std::move(c));
            },
            PickOverload<
                std::string, std::vector<std::string>, std::function<void(std::string const&, size_t)>, size_t>(
                &OverlayWindow::add_dropdown
            )
        );
        overlay_window["add_color_picker"] = &OverlayWindow::add_color_picker;
        overlay_window["add_texture"] = sol::overload(
            PickOverload<Texture2D const&, Vector2ui const&>(&OverlayWindow::add_texture),
            PickOverload<Texture2D const&>(&OverlayWindow::add_texture)
        );
        overlay_window["add_progress_bar"] = sol::overload(
            [](OverlayWindow& w, Vector2i const& range) { return &w.add_progress_bar(range); },
            PickOverload<Vector2i const&, bool>(&OverlayWindow::add_progress_bar)
        );
        overlay_window["add_plot"] = sol::overload(
            [](OverlayWindow& w, std::string l, size_t c) { return &w.add_plot(std::move(l), c); },
            [](OverlayWindow& w, std::string l, size_t c, std::string x) {
                return &w.add_plot(std::move(l), c, std::move(x));
            },
            [](OverlayWindow& w, std::string l, size_t c, std::string x, std::string y) {
                return &w.add_plot(std::move(l), c, std::move(x), std::move(y));
            },
            [](OverlayWindow& w, std::string l, size_t c, std::string x, std::string y, Vector2f range) {
                return &w.add_plot(std::move(l), c, std::move(x), std::move(y), std::move(range));
            },
            [](OverlayWindow& w, std::string l, size_t c, std::string x, std::string y, Vector2f range, bool lock) {
                return &w.add_plot(std::move(l), c, std::move(x), std::move(y), std::move(range), lock);
            },
            PickOverload<std::string, size_t, std::string, std::string, Vector2f, bool, float>(&OverlayWindow::add_plot)
        );
        overlay_window["add_separator"] = &OverlayWindow::add_separator;
        overlay_window["add_frame_time"] = &OverlayWindow::add_frame_time;
        overlay_window["add_fps_counter"] = &OverlayWindow::add_fps_counter;
        overlay_window["render"] = &OverlayWindow::render;
    }
}
}