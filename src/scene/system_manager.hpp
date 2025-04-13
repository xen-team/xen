#pragma once

#include "system.hpp"

#include "utils/classes.hpp"
#include "debug/log.hpp"

#include <memory>

namespace xen {
class XEN_API SystemManager : NonCopyable {
public:
    template <SystemDerived S>
    bool has() const
        requires(std::is_convertible_v<S*, System*>)
    {
        auto const it = systems.find(TypeInfo<System>::GetTypeId<S>());

        return it != systems.end() && it->second;
    }

    template <SystemDerived S>
    S* get() const
        requires(std::is_convertible_v<S*, System*>)
    {
        auto it = systems.find(TypeInfo<System>::GetTypeId<S>());

        if (it == systems.end() || !it->second) {
            // throw std::runtime_error("Scene does not have requested System");
            return nullptr;
        }

        return static_cast<S*>(it->second.get());
    }

    template <SystemDerived S>
    void add(std::unique_ptr<S>&& system)
        requires(std::is_convertible_v<S*, System*>)
    {
        // Remove previous System, if it exists.
        remove<S>();

        auto const typeId = TypeInfo<System>::GetTypeId<S>();

        // Then, add the System
        systems[typeId] = std::move(system);
    }

    template <SystemDerived S>
    void remove()
        requires(std::is_convertible_v<System*, System*>)
    {
        auto const typeId = TypeInfo<System>::GetTypeId<S>();

        // Then, remove the System.
        systems.erase(typeId);
    }

    void clear();

    template <typename Func>
    void for_each(Func&& func)
    {
        for (auto& [typeId, system] : systems) {
            try {
                func(typeId, system.get());
            }
            catch (std::exception const& e) {
                Log::error(e.what(), '\n');
            }
        }
    }

private:
    /// List of all Systems.
    std::unordered_map<TypeId, std::unique_ptr<System>> systems;
};
}