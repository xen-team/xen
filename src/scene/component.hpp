#pragma once

#include <utility>
#include <cereal/archives/portable_binary.hpp>
#include <entt.hpp>

namespace xen {

template <typename Archive, typename Type>
void save(Archive& archive, entt::registry const& reg)
{
    entt::snapshot{reg}.template get<Type>(archive);
}

template <typename Archive, typename Type>
void load(Archive& archive, entt::registry& reg)
{
    entt::snapshot_loader{reg}.template get<Type>(archive);
}

template <
    typename T, typename ArchiveOut = cereal::PortableBinaryOutputArchive,
    typename ArchiveIn = cereal::PortableBinaryInputArchive>
constexpr static bool register_component()
{
    using namespace entt::literals;

    // entt::meta<T>().type("name"_hs).template func<&save<ArchiveOut, T>>("save"_hs).template func<&load<ArchiveIn,
    // T>>(
    //     "load"_hs
    // );

    return true;
}

class Component {
private:
    bool started = false;
    bool enabled = true;

public:
    virtual ~Component() = default;

    virtual void start() {};

    virtual void update() {};

    [[nodiscard]] bool is_enabled() const { return enabled; }
    void set_enabled(bool enable) { this->enabled = enable; }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(started, enabled);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(started, enabled);
    }
};
}

namespace cereal {
template <class Archive>
void save(Archive& archive, entt::registry const& reg)
{
    using namespace entt;
    for (auto&& storage : reg.storage()) {
        auto type = entt::resolve(storage.second.type());
        type.invoke("save"_hs, {}, entt::forward_as_meta(archive), entt::forward_as_meta(reg));
    }
}

template <class Archive>
void load(Archive& archive, entt::registry& reg)
{
    using namespace entt;
    for (auto&& storage : reg.storage()) {
        auto type = entt::resolve(storage.second.type());
        type.invoke("load"_hs, {}, entt::forward_as_meta(archive), entt::forward_as_meta(reg));
    }
}
}