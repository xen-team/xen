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

        friend void to_json(nlohmann::json& j, InputSchemeData const& data)
        {
            to_json(j["buttons"], data.buttons);
            to_json(j["axes"], data.axes);
        }
        friend void from_json(nlohmann::json const& j, InputSchemeData& data)
        {
            from_json(j["buttons"], data.buttons);
            from_json(j["axes"], data.axes);
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

    void read()
    {
        Windows::get()->get_focused_window()->on_mouse_pos.clear();
        file.read(data);
    }

private:
    void move_signals(InputScheme* other);
};
}