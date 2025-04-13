#pragma once

#include <scene/components/camera.hpp>

namespace xen {
class DebugCamera : public CameraComponent {
    inline static bool const registered = register_component<DebugCamera>();

public:
    DebugCamera();

    void start() override;
    void update() override;
};
}