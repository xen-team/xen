#pragma once

#include "core.hpp"
#include "debug/log.hpp"

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

namespace xen {
template <typename Base, typename... Args>
class JsonFactory {
public:
    using TCreateReturn = std::unique_ptr<Base>;

    using TCreateMethod = std::function<TCreateReturn(Args...)>;
    using TRegistryMap = std::unordered_map<std::string, TCreateMethod>;

    virtual ~JsonFactory() = default;

    static auto& registry()
    {
        static TRegistryMap impl;
        return impl;
    }

    static TCreateReturn create(std::string const& name, Args&&... args)
    {
        auto it = registry().find(name);
        if (it == registry().end()) {
            Log::error("Failed to create ", std::quoted(name), " from factory\n");
            return nullptr;
        }
        return it->second(std::forward<Args>(args)...);
    }

    static TCreateReturn create(nlohmann::json const& j)
    {
        auto name = j["type"].template get<std::string>();
        auto it = registry().find(name);
        if (it == registry().end()) {
            Log::error("Failed to create ", std::quoted(name), " from factory\n");
            return nullptr;
        }
        return it->second();
    }

    [[nodiscard]] virtual std::string get_type_name() const { return ""; }

    template <typename U>
    static void save_value(nlohmann::json& j, std::string_view name, U const& v)
    {
        j[name] = v;
    }

    template <typename U>
    static void load_value(nlohmann::json const& j, std::string_view name, U& v)
    {
        j.at(name).get_to(v);
    }

    virtual void save(nlohmann::json& j) { base_save(j); }
    virtual void load(nlohmann::json const& j) { base_load(j); }

    template <typename T>
    class Registrar : public Base {
    protected:
        inline static std::string name;

    public:
        [[nodiscard]] std::string get_type_name() const override { return name; }

        void save_base(nlohmann::json& j)
        {
            Base::base_save(j);
            j["type"] = name;
        }
        void load_base(nlohmann::json const& j) { Base::base_load(j); }

    protected:
        static bool Register(std::string const& name)
        {
            Registrar::name = name;
            JsonFactory::registry()[name] = [](Args... args) -> TCreateReturn {
                return std::make_unique<T>(std::forward<Args>(args)...);
            };
            return true;
        }

        TCreateReturn create_by_name(std::string const& name, Args&&... args)
        {
            return JsonFactory<Base>::create(name, args...);
        }
        TCreateReturn create_by_type(nlohmann::json const& j) { return JsonFactory<Base>::create(j); }

        template <typename U>
        static void save_value(nlohmann::json& j, std::string_view name, U const& v)
        {
            j[name] = v;
        }

        template <typename U>
        static void load_value(nlohmann::json const& j, std::string_view name, U& v)
        {
            j.at(name).get_to(v);
        }

        template <typename U>
        static U get_value(nlohmann::json const& j, std::string_view name)
        {
            j.at(name).template get<U>();
        }
    };

protected:
    virtual void base_save(nlohmann::json& j);
    virtual void base_load(nlohmann::json const& j);
};
}