#pragma once

namespace sol {
template <bool>
class basic_reference;
using reference = basic_reference<false>;

template <typename>
struct basic_environment;
using environment = basic_environment<reference>;

}

namespace xen {
class Entity;
class FilePath;

class LuaEnvironment {
    friend class LuaScript;

public:
    LuaEnvironment();
    LuaEnvironment(LuaEnvironment const&) = delete;
    LuaEnvironment(LuaEnvironment&&) noexcept = default;

    LuaEnvironment& operator=(LuaEnvironment const&) = delete;
    LuaEnvironment& operator=(LuaEnvironment&&) noexcept = default;

    ~LuaEnvironment();

    /// Executes a script from a string.
    /// \param code Code to be executed.
    /// \return True if the script has been executed without errors, false otherwise.
    bool execute(std::string const& code) const;

    /// Executes a script from a file.
    /// \param filepath Path to the script to be executed. Must be valid ASCII; special characters are not handled.
    /// \return True if the script has been executed without errors, false otherwise.
    bool execute_from_file(FilePath const& filepath) const;

    /// Checks if a given global symbol (variable or function) is registered in the environment.
    /// \note Symbols declared as 'local' will NOT be found by this call, as they exist only during the script's
    /// execution.
    /// \param name Name of the symbol to be checked.
    /// \return True if the symbol has been found, false otherwise.
    bool exists(char const* name) const;

    /// Clears the environment, removing all existing symbols.
    void clear();

private:
    std::unique_ptr<sol::environment> environment;

private:
    /// Registers an entity to a variable, making it accessible from the environment.
    /// \param entity Entity to be registered.
    /// \param name Name of the variable to bind the entity to.
    void register_entity(Entity const& entity, std::string const& name);

    /// Gets an abstract object to a symbol.
    /// \param name Name of the symbol to get the object from.
    /// \return Reference object tied to the symbol.
    sol::reference get(char const* name) const;
};
}