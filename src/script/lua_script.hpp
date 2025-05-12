#pragma once

#include <component.hpp>
#include <script/lua_environment.hpp>

namespace xen {

class FilePath;
struct FrameTimeInfo;

class LuaScript final : public Component {
    friend class ScriptSystem;

public:
    explicit LuaScript(std::string const& code);

    LuaEnvironment const& get_environment() const { return environment; }

    /// Loads a script from a string.
    /// \note The script must contain a function named update().
    /// \note This clears the script's environment, effectively unregistering all existing symbols.
    /// \param code Code to be loaded.
    void load_code(std::string const& code);

    /// Loads a script from a file.
    /// \note The script must contain a function named update().
    /// \note This clears the script's environment, effectively unregistering all existing symbols.
    /// \param filepath Path to the script to be loaded.
    void load_code_from_file(FilePath const& filepath);

    /// Registers an entity to a variable, making it accessible from the script.
    /// \param entity Entity to be registered.
    /// \param name Name of the variable to bind the entity to.
    void register_entity(Entity const& entity, std::string const& name) { environment.register_entity(entity, name); }

    /// Executes the script's update function.
    /// \warning If the update function does not return anything or returns nil, this call will always return false.
    /// \warning If the update function does return anything other than a boolean, this call will always return true.
    /// \param time_info Time-related frame information.
    /// \retval true If the update function returned true or any non-boolean value (excluding nil).
    /// \retval false If the update function returned false, nil or nothing.
    bool update(FrameTimeInfo const& time_info) const;

private:
    LuaEnvironment environment{};

private:
    /// Executes the script's setup function. Does nothing if none exists.
    /// \throws std::runtime_error If the setup function has not been executed correctly.
    void setup() const;
};
}