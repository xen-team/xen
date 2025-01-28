#include "input_scheme.hpp"
#include "debug/log.hpp"

namespace xen {
InputScheme::InputScheme(std::filesystem::path const& filename) : file{filename}
{
    // file.read(data);
}

InputAxis* InputScheme::get_axis(std::string const& name)
{
    auto& axes = data.axes;

    auto [it, success] = axes.try_emplace(name, std::make_unique<InputAxis>());
    if (success) {
        Log::warning("InputAxis was not found in input scheme: ", std::quoted(name), '\n');
    }
    return it->second.get();
}

InputAxis* InputScheme::add_axis(std::string const& name, std::unique_ptr<InputAxis>&& axis)
{
    return data.axes.emplace(name, std::move(axis)).first->second.get();
}

void InputScheme::remove_axis(std::string const& name)
{
    auto& axes = data.axes;

    if (auto it = axes.find(name); it != axes.end()) {
        axes.erase(it);
    }
    else {
        Log::warning("InputAxis was not found in input scheme: ", std::quoted(name), '\n');
    }
}

InputButton* InputScheme::get_button(std::string const& name)
{
    auto& buttons = data.buttons;
    if (auto it = buttons.find(name); it != buttons.end()) {
        return it->second.get();
    }

    Log::error("InputButton was not found in input scheme: ", std::quoted(name), '\n');
    return nullptr;
}

InputButton* InputScheme::add_button(std::string const& name, std::unique_ptr<InputButton>&& button)
{
    return data.buttons.emplace(name, std::move(button)).first->second.get();
}

void InputScheme::remove_button(std::string const& name)
{
    auto& buttons = data.buttons;

    if (auto it = buttons.find(name); it != buttons.end()) {
        buttons.erase(it);
    }
    else {
        Log::warning("InputButton was not found in input scheme: ", std::quoted(name), '\n');
    }
}

void InputScheme::move_signals(InputScheme* other)
{
    if (!other) {
        return;
    }

    auto& [axes, buttons] = data;
    auto& [other_axes, other_buttons] = other->data;

    for (auto& [name, axis] : other_axes) {
        if (auto it = axes.find(name); it != axes.end()) {
            std::swap(it->second->on_axis, axis->on_axis);
        }
        else {
            Log::warning("InputAxis was not found in input scheme: ", std::quoted(name), '\n');
        }
    }

    for (auto& [name, button] : other_buttons) {
        if (auto it = buttons.find(name); it != buttons.end()) {
            std::swap(it->second->on_button, button->on_button);
        }
        else {
            Log::warning("InputButton was not found in input scheme: ", std::quoted(name), '\n');
        }
    }
}
}