#include "inputs.hpp"
#include "debug/log.hpp"
#include <iomanip>

namespace xen {
void Inputs::update() {}

InputScheme* Inputs::get_scheme(std::string const& name) const
{
    if (auto it = schemes.find(name); it != schemes.end()) {
        return it->second.get();
    }

    Log::error("Could not find input scheme: ", std::quoted(name), '\n');
    return nullptr;
}

InputScheme* Inputs::add_scheme(std::string const& name, std::unique_ptr<InputScheme>&& scheme, bool set_current)
{
    InputScheme* new_scheme = schemes.emplace(name, std::move(scheme)).first->second.get();
    if (set_current) {
        set_scheme(new_scheme);
    }

    return new_scheme;
}

void Inputs::remove_scheme(std::string const& name)
{
    auto it = schemes.find(name);
    if (current_scheme == it->second.get() && schemes.size() > 1) {
        set_scheme(schemes.begin()->second.get());
    }
    if (it != schemes.end()) {
        schemes.erase(it);
    }
}

void Inputs::set_scheme(InputScheme* scheme)
{
    if (!scheme) {
        return;
    }

    // We want to preserve signals from the current scheme to the new one.
    scheme->move_signals(current_scheme);
    current_scheme = scheme;
}

void Inputs::set_scheme(std::string const& name)
{
    InputScheme* scheme = get_scheme(name);
    if (!scheme) {
        return;
    }

    set_scheme(scheme);
}

InputAxis* Inputs::get_axis(std::string const& name) const
{
    return current_scheme ? current_scheme->get_axis(name) : nullptr;
}

InputButton* Inputs::get_button(std::string const& name) const
{
    return current_scheme ? current_scheme->get_button(name) : nullptr;
}
}