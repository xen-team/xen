#pragma once

namespace xen {
class FrameBuffer;

class RenderBuffer {
    using BindableId = unsigned int;

    BindableId id = 0;
    int width = 0, height = 0, samples = 0;

public:
    RenderBuffer();
    ~RenderBuffer();
    RenderBuffer(RenderBuffer const&) = delete;
    RenderBuffer(RenderBuffer&&) noexcept;
    RenderBuffer& operator=(RenderBuffer const&) = delete;
    RenderBuffer& operator=(RenderBuffer&&) noexcept;

    BindableId get_native_handle() const { return id; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_samples() const { return samples; }

    void init_storage(int width, int height, int samples = 0);

    void link_to_framebuffer(FrameBuffer const& framebuffer) const;

    void bind() const;

    void unbind() const;

private:
    void free();
};
}