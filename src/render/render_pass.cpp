#include "render_pass.hpp"

#include <render/renderer.hpp>
#include <render/texture.hpp>

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
bool RenderPass::is_valid() const
{
    // Since a pass can get read & write buffers from other sources than the previous pass, one may have more or less
    //  buffers than its parent write to. Direct buffer compatibility is thus not checked

    std::vector<std::pair<Texture2DPtr, uint>> const& write_color_buffers = write_framebuffer.color_buffers;

    for (auto const& [texture, _] : program.get_textures()) {
        // If the same depth buffer exists both in read & write, the pass is invalid
        if (texture->get_colorspace() == TextureColorspace::DEPTH && write_framebuffer.has_depth_buffer()) {
            if (texture.get() == &write_framebuffer.get_depth_buffer()) {
                return false;
            }
        }

        auto const buffer_it = std::find_if(
            write_color_buffers.cbegin(), write_color_buffers.cend(),
            [&read_texture = texture](auto const& buffer) { return (read_texture == buffer.first); }
        );

        // Likewise for the color buffers: if any has been added as both read & write, the pass is invalid
        if (buffer_it != write_color_buffers.cend()) {
            return false;
        }
    }

    return true;
}

void RenderPass::add_read_texture(TexturePtr texture, std::string const& uniform_name)
{
    program.set_texture(std::move(texture), uniform_name);
    program.init_textures();
}

void RenderPass::execute() const
{
    ZoneScopedN("RenderPass::execute");

    if (!enabled) {
        return;
    }

    TracyGpuZoneTransient(_, (name.empty() ? "[Unnamed pass]" : name.c_str()), true)

#if !defined(USE_OPENGL_ES)
#if defined(XEN_CONFIG_DEBUG)
        if (Renderer::check_version(4, 3) && !name.empty())
    {
        Renderer::push_debug_group(name);
    }
#endif

    timer.start();
#endif

    // Binding the program's textures marks it as used
    program.bind_textures();

    if (!write_framebuffer.empty()) {
        write_framebuffer.bind();
    }
    write_framebuffer.display();
    write_framebuffer.unbind();

#if !defined(USE_OPENGL_ES)
    timer.stop();

#if defined(XEN_CONFIG_DEBUG)
    if (Renderer::check_version(4, 3) && !name.empty()) {
        Renderer::pop_debug_group();
    }
#endif
#endif
}

}
