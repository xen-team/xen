#pragma once

#include "input_scheme.hpp"

namespace xen {
class XEN_API Inputs : public Module::Registrar<Inputs> {
    inline static bool const registered = Register(Stage::Pre, Depends<Windows, Joysticks>());

private:
    std::map<std::string, std::unique_ptr<InputScheme>> schemes;
    InputScheme* current_scheme = nullptr;

public:
    void update() override;

    [[nodiscard]] InputScheme* get_scheme() const { return current_scheme; }
    [[nodiscard]] InputScheme* get_scheme(std::string const& name) const;
    [[nodiscard]] InputScheme*
    add_scheme(std::string const& name, std::unique_ptr<InputScheme>&& scheme, bool set_current = false);
    void remove_scheme(std::string const& name);
    void set_scheme(InputScheme* scheme);
    void set_scheme(std::string const& name);

    [[nodiscard]] InputAxis* get_axis(std::string const& name) const;
    [[nodiscard]] InputButton* get_button(std::string const& name) const;
};
}