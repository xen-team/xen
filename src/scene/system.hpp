#pragma once

#include "utils/classes.hpp"
#include "utils/type_info.hpp"
#include "core.hpp"

namespace xen {
class Scene;

class XEN_API System : NonCopyable {
    friend class Scene;

public:
    virtual ~System() = default;

    virtual void start() = 0;

    virtual void update() = 0;

    [[nodiscard]] bool is_enabled() const { return enabled; }
    void set_enabled(bool enable) { this->enabled = enable; }

private:
    bool enabled = true;

protected:
    Scene* scene = nullptr;
};

template <typename T>
concept SystemDerived = std::is_base_of_v<System, T>;

template class XEN_API TypeInfo<System>;
}