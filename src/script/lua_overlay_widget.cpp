#include <render/overlay.hpp>
#include <render/texture.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_overlay_widget_types()
{
    sol::state& state = get_state();

    {
        state.new_usertype<OverlayButton>(
            "OverlayButton", sol::constructors<OverlayButton(std::string, std::function<void()>)>(), sol::base_classes,
            sol::bases<OverlayElement>()
        );
    }

    {
        state.new_usertype<OverlayCheckbox>(
            "OverlayCheckbox",
            sol::constructors<OverlayCheckbox(std::string, std::function<void()>, std::function<void()>, bool)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
    }

    {
        sol::usertype<OverlayColoredLabel> overlay_colored_label = state.new_usertype<OverlayColoredLabel>(
            "OverlayColoredLabel",
            sol::constructors<
                OverlayColoredLabel(std::string, Color const&), OverlayColoredLabel(std::string, Color const&, float)>(
            ),
            sol::base_classes, sol::bases<OverlayElement>()
        );
        overlay_colored_label["text"] =
            sol::property(&OverlayColoredLabel::get_label, PickOverload<std::string>(&OverlayColoredLabel::set_label));
        overlay_colored_label["color"] =
            sol::property(&OverlayColoredLabel::get_color, [](OverlayColoredLabel& l, Color const& c) {
                l.set_color(c);
            });
        overlay_colored_label["alpha"] =
            sol::property(&OverlayColoredLabel::get_alpha, &OverlayColoredLabel::set_alpha);
    }

    {
        state.new_usertype<OverlayColorPicker>(
            "OverlayColorPicker",
            sol::constructors<OverlayColorPicker(std::string, std::function<void(Color const&)>, Color const&)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
    }

    {
        state.new_usertype<OverlayDropdown>(
            "OverlayDropdown",
            sol::constructors<
                OverlayDropdown(std::string, std::vector<std::string>, std::function<void(std::string const&, size_t)>),
                OverlayDropdown(
                    std::string, std::vector<std::string>, std::function<void(std::string const&, size_t)>, size_t
                )>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
    }

    {
        sol::usertype<OverlayElement> overlay_element =
            state.new_usertype<OverlayElement>("OverlayElement", sol::no_constructor);
        overlay_element["get_type"] = &OverlayElement::get_type;
        overlay_element["is_enabled"] = &OverlayElement::is_enabled;
        overlay_element["enable"] =
            sol::overload([](OverlayElement& r) { r.enable(); }, PickOverload<bool>(&OverlayElement::enable));
        overlay_element["disable"] = &OverlayElement::disable;
    }

    {
        state.new_usertype<OverlayFpsCounter>(
            "OverlayFpsCounter", sol::constructors<OverlayFpsCounter(std::string)>(), sol::base_classes,
            sol::bases<OverlayElement>()
        );
    }

    {
        state.new_usertype<OverlayFrameTime>(
            "OverlayFrameTime", sol::constructors<OverlayFrameTime(std::string)>(), sol::base_classes,
            sol::bases<OverlayElement>()
        );
    }

    {
        sol::usertype<OverlayLabel> overlay_label = state.new_usertype<OverlayLabel>(
            "OverlayLabel", sol::constructors<OverlayLabel(std::string)>(), sol::base_classes,
            sol::bases<OverlayElement>()
        );
        overlay_label["text"] = sol::property(&OverlayLabel::get_label, &OverlayLabel::set_label);
    }

    {
        state.new_usertype<OverlayListBox>(
            "OverlayListBox",
            sol::constructors<
                OverlayListBox(std::string, std::vector<std::string>, std::function<void(std::string const&, size_t)>),
                OverlayListBox(
                    std::string, std::vector<std::string>, std::function<void(std::string const&, size_t)>, size_t
                )>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
    }

    // OverlayPlot
    {
        {
            sol::usertype<OverlayPlot> overlay_plot = state.new_usertype<OverlayPlot>(
                "OverlayPlot",
                sol::constructors<
                    OverlayPlot(std::string, size_t), OverlayPlot(std::string, size_t, std::string),
                    OverlayPlot(std::string, size_t, std::string, std::string),
                    OverlayPlot(std::string, size_t, std::string, std::string, Vector2f),
                    OverlayPlot(std::string, size_t, std::string, std::string, Vector2f, bool),
                    OverlayPlot(std::string, size_t, std::string, std::string, Vector2f, bool, float)>(),
                sol::base_classes, sol::bases<OverlayElement>()
            );
            overlay_plot["add_entry"] = sol::overload(
                [](OverlayPlot& p, std::string n) { return p.add_entry(std::move(n)); },
                PickOverload<std::string, OverlayPlotType>(&OverlayPlot::add_entry)
            );
        }

        {
            sol::usertype<OverlayPlotEntry> overlay_plot_entry =
                state.new_usertype<OverlayPlotEntry>("OverlayPlotEntry", sol::no_constructor);
            overlay_plot_entry["push"] = &OverlayPlotEntry::push;
        }

        state.new_enum<OverlayPlotType>(
            "OverlayPlotType", {{"LINE", OverlayPlotType::LINE}, {"SHADED", OverlayPlotType::SHADED}}
        );
    }

    {
        sol::usertype<OverlayProgressBar> overlay_progress_bar = state.new_usertype<OverlayProgressBar>(
            "OverlayProgressBar",
            sol::constructors<OverlayProgressBar(Vector2ui const&), OverlayProgressBar(Vector2ui const&, bool)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
        overlay_progress_bar["current_value"] =
            sol::property(&OverlayProgressBar::get_value, &OverlayProgressBar::set_value);
        overlay_progress_bar["has_started"] = &OverlayProgressBar::has_started;
        overlay_progress_bar["has_finished"] = &OverlayProgressBar::has_finished;
        overlay_progress_bar["add"] = &OverlayProgressBar::operator+=;
    }

    {
        state.new_usertype<OverlaySeparator>(
            "OverlaySeparator", sol::constructors<OverlaySeparator()>(), sol::base_classes, sol::bases<OverlayElement>()
        );
    }

    {
        state.new_usertype<OverlaySlider>(
            "OverlaySlider",
            sol::constructors<OverlaySlider(std::string, std::function<void(float)>, Vector2f const&, float)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
    }

    {
        sol::usertype<OverlayTextArea> overlay_text_area = state.new_usertype<OverlayTextArea>(
            "OverlayTextArea",
            sol::constructors<
                OverlayTextArea(std::string, std::function<void(std::string const&)>),
                OverlayTextArea(std::string, std::function<void(std::string const&)>, std::string),
                OverlayTextArea(std::string, std::function<void(std::string const&)>, std::string, float)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
        overlay_text_area["text"] = sol::property(&OverlayTextArea::get_label, &OverlayTextArea::set_label);
        overlay_text_area["append"] = &OverlayTextArea::append;
        overlay_text_area["clear"] = &OverlayTextArea::clear;
    }

    {
        sol::usertype<OverlayTextbox> overlay_textbox = state.new_usertype<OverlayTextbox>(
            "OverlayTextbox",
            sol::constructors<
                OverlayTextbox(std::string, std::function<void(std::string const&)>),
                OverlayTextbox(std::string, std::function<void(std::string const&)>, std::string)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
        overlay_textbox["text"] = sol::property(&OverlayTextbox::get_label, &OverlayTextbox::set_label);
        overlay_textbox["append"] = &OverlayTextbox::append;
        overlay_textbox["clear"] = &OverlayTextbox::clear;
    }

    {
        sol::usertype<OverlayTexture> overlay_texture = state.new_usertype<OverlayTexture>(
            "OverlayTexture",
            sol::constructors<OverlayTexture(Texture2D const&, Vector2ui const&), OverlayTexture(Texture2D const&)>(),
            sol::base_classes, sol::bases<OverlayElement>()
        );
        overlay_texture["set_texture"] = sol::overload(
            PickOverload<Texture2D const&, Vector2ui const&>(&OverlayTexture::set_texture),
            PickOverload<Texture2D const&>(&OverlayTexture::set_texture)
        );
    }

    state.new_enum<OverlayElementType>(
        "OverlayElementType", {{"LABEL", OverlayElementType::LABEL},
                               {"COLORED_LABEL", OverlayElementType::COLORED_LABEL},
                               {"BUTTON", OverlayElementType::BUTTON},
                               {"CHECKBOX", OverlayElementType::CHECKBOX},
                               {"SLIDER", OverlayElementType::SLIDER},
                               {"TEXTBOX", OverlayElementType::TEXTBOX},
                               {"TEXT_AREA", OverlayElementType::TEXT_AREA},
                               {"LIST_BOX", OverlayElementType::LIST_BOX},
                               {"DROPDOWN", OverlayElementType::DROPDOWN},
                               {"COLOR_PICKER", OverlayElementType::COLOR_PICKER},
                               {"TEXTURE", OverlayElementType::TEXTURE},
                               {"PROGRESS_BAR", OverlayElementType::PROGRESS_BAR},
                               {"PLOT", OverlayElementType::PLOT},
                               {"SEPARATOR", OverlayElementType::SEPARATOR},
                               {"FRAME_TIME", OverlayElementType::FRAME_TIME},
                               {"FPS_COUNTER", OverlayElementType::FPS_COUNTER}}
    );
}
}