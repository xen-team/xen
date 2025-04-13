#pragma once

#include "core.hpp"
#include "math/vector2.hpp"

#include <vector>

namespace xen {
class PhysicalDevice;
class Surface;
class LogicalDevice;
class Window;

class XEN_API Swapchain {
private:
    std::shared_ptr<Window> window;
    Vector2ui size;

    uint32_t framebuffer;
    uint32_t renderbuffer;
    std::vector<uint32_t> textures;

public:
    Swapchain(std::shared_ptr<Window> window, Vector2ui size);
    ~Swapchain();

    void bind();

    void swap_buffers();

    void acquire_next_image();

    void queue_present();

    [[nodiscard]] bool is_same_extent(Vector2ui const& size) { return this->size == size; }

    [[nodiscard]] Vector2ui get_size() const { return size; }
};
}