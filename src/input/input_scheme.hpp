#pragma once

#include "core.hpp"
#include "joysticks.hpp"
#include "files/json.hpp"
#include "utils/classes.hpp"
#include "input_axis.hpp"
#include "input_button.hpp"
#include "utils/json_factory.hpp"

namespace xen {
class XEN_API InputScheme : NonCopyable {
    friend class Inputs;

public:
    using AxisMap = std::map<std::string, std::unique_ptr<InputAxis>>;
    using ButtonMap = std::map<std::string, std::unique_ptr<InputButton>>;
    using JoystickMap = std::map<std::string, JoystickPort>;

    struct InputSchemeData {
        AxisMap axes;
        ButtonMap buttons;

        void save(nlohmann::json& j)
        {
            for (auto& [name, button] : buttons) {
                button->save(j[name]);
            }
        }
        void load(nlohmann::json const& j)
        {
            for (auto const& [key, value] : j.items()) {
                auto button = buttons.emplace(key, JsonFactory<InputButton>::create(value));
                button.first->second->load(j[key]);
            }
        }
    };

private:
    InputSchemeData data;
    JsonFile file;

public:
    explicit InputScheme(std::filesystem::path const& filename);

    [[nodiscard]] InputAxis* get_axis(std::string const& name);
    InputAxis* add_axis(std::string const& name, std::unique_ptr<InputAxis>&& axis);
    void remove_axis(std::string const& name);

    [[nodiscard]] InputButton* get_button(std::string const& name);
    InputButton* add_button(std::string const& name, std::unique_ptr<InputButton>&& button);
    void remove_button(std::string const& name);

    [[nodiscard]] JsonFile& get_file() { return file; }

    void write() { file.write(data); }

    void read() { file.read(data); }

private:
    void move_signals(InputScheme* other);
};
}