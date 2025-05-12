#pragma once

namespace xen {
class Component;
using ComponentPtr = std::unique_ptr<Component>;

/// Component class representing a base Component to be inherited.
class Component {
public:
    /// Gets the ID of the given component type.
    /// It uses CRTP to assign a different ID to each component type it is called with.
    /// This function will be instantiated every time it is called with a different type, incrementing the assigned
    /// index. Note that it must be called directly from Component, and a derived class must be given
    /// (Component::get_id<DerivedComponent>()).
    /// \tparam CompT Type of the component to get the ID of.
    /// \return Given component's ID.
    template <typename CompT>
    static size_t get_id();

    virtual ~Component() = default;

protected:
    Component() = default;
    Component(Component const&) = default;
    Component(Component&&) noexcept = default;

    Component& operator=(Component const&) = default;
    Component& operator=(Component&&) noexcept = default;

private:
    static inline size_t max_id = 0;
};
}

#include "component.inl"