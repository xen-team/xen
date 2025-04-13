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

    template <typename T>
    class Registrar : public Base {
    protected:
        inline static std::string name;

    public:
        [[nodiscard]] std::string get_type_name() const override { return name; }

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

        void raw_save(nlohmann::json& j) const override
        {
            j["type"] = name;
            j = *dynamic_cast<T const*>(this);
        }

        void raw_load(nlohmann::json const& j) override { j.at(name).get_to(*dynamic_cast<T*>(this)); }
    };

    friend void save(nlohmann::json& j, std::unique_ptr<Base> const& object) { object->raw_save(j); }

    friend void load(nlohmann::json const& j, std::unique_ptr<Base>& object)
    {
        if (j.contains("type")) {
            object = create(j["type"].template get<std::string>());
        }
        load(j, *object);
    }

    friend void save(nlohmann::json& j, Base const& base) { base.save(j); }
    friend void load(nlohmann::json const& j, Base& base) { return base.load(j); }

protected:
    virtual void raw_save(nlohmann::json& j) const { ; }
    virtual void raw_load(nlohmann::json const& j) { ; }
};
}