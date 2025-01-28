#pragma once

#include "core.hpp"
#include <rocket.hpp>

namespace xen {
class XEN_API App : public virtual rocket::trackable {
    friend class Engine;

public:
    explicit App(std::string name) : name(std::move(name)) {}

    virtual ~App() = default;

    virtual void start() = 0;

    virtual void update() = 0;

    [[nodiscard]] std::string const& get_name() const { return name; }

    void set_name(std::string const& name) { this->name = name; }

private:
    std::string name;
    bool started = false;
};
}