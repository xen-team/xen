#pragma once

#include "system/windows.hpp"

namespace xen {
class XEN_API Render : public Module::Registrar<Render> {
    inline static bool const registered = Register(Stage::Render, Depends<Windows>());

public:
    Render();
    ~Render();

    void update() override;
};
}