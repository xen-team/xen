#include "render_stage.hpp"

#include "system/windows.hpp"
#include "render.hpp"

namespace xen {
RenderStage::RenderStage(std::vector<Attachment> images, std::vector<SubpassType> subpasses, Viewport const& viewport) :
    attachments(std::move(images)), subpasses(std::move(subpasses)), viewport(viewport),
    subpass_attachment_count(this->subpasses.size()), subpass_multisampled(this->subpasses.size())
{
    for (auto const& image : attachments) {
        ClearValueType clear_value = {0.0f, 0.0f, 0.0f, 1.0f};

        switch (image.get_type()) {
        case Attachment::Type::Image:
            clear_value[0] = image.get_clear_color().r;
            clear_value[1] = image.get_clear_color().g;
            clear_value[2] = image.get_clear_color().b;
            clear_value[3] = image.get_clear_color().a;

            for (auto const& subpass : this->subpasses) {
                if (auto subpass_bindings = subpass.get_attachment_bindings();
                    std::find(subpass_bindings.begin(), subpass_bindings.end(), image.get_binding()) !=
                    subpass_bindings.end()) {
                    subpass_attachment_count[subpass.get_binding()]++;

                    if (image.is_multisampled())
                        subpass_multisampled[subpass.get_binding()] = true;
                }
            }

            break;
        case Attachment::Type::Depth:
            depth_attachment = image;
            break;
        case Attachment::Type::Swapchain:
            clear_value[0] = image.get_clear_color().r;
            clear_value[1] = image.get_clear_color().g;
            clear_value[2] = image.get_clear_color().b;
            clear_value[3] = image.get_clear_color().a;

            swapchain_attachment = image;
            break;
        }

        clear_values.emplace_back(clear_value);
    }
}

void RenderStage::update()
{
    auto last_render_area = render_area;

    render_area.set_offset(viewport.get_offset());

    if (viewport.get_size()) {
        render_area.set_extent(viewport.get_scale() * *viewport.get_size());
    }
    else {
        render_area.set_extent(viewport.get_scale() * Windows::get()->get_window(0)->get_size());
    }

    render_area.set_aspect_ratio(
        static_cast<float>(render_area.get_extent().x) / static_cast<float>(render_area.get_extent().y)
    );
    render_area.set_extent(render_area.get_extent() + render_area.get_offset());

    out_of_date = render_area != last_render_area;
}

void RenderStage::rebuild(Swapchain const& swapchain)
{
#ifdef XEN_DEBUG
    auto debug_start = Time::now();
#endif

    update();

    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();
    auto surface = Graphics::Get()->GetSurface(0);

    auto msaaSamples = physicalDevice->GetMsaaSamples();

    if (depth_attachment)
        depthStencil = std::make_unique<ImageDepth>(
            renderArea.GetExtent(), depthAttachment->IsMultisampled() ? msaaSamples : VK_SAMPLE_COUNT_1_BIT
        );

    if (!renderpass)
        renderpass = std::make_unique<Renderpass>(
            *this, depth_stencil ? depth_stencil->GetFormat() : VK_FORMAT_UNDEFINED, surface->GetFormat().format,
            msaaSamples
        );

    framebuffers = std::make_unique<Framebuffers>(
        swapchain, *this, *renderpass, *depth_stencil, renderArea.GetExtent(), msaaSamples
    );

    out_of_date = false;

#ifdef XEN_DEBUG
    Log::Out("Render Stage created in ", (Time::now() - debug_start).as_milliseconds<float>(), "ms\n");
#endif
}

std::optional<Attachment> RenderStage::get_attachment(std::string const& name) const
{
    auto it = std::ranges::find_if(attachments, [name](Attachment const& a) { return a.get_name() == name; });
    if (it != attachments.end()) {
        return *it;
    }

    return std::nullopt;
}

std::optional<Attachment> RenderStage::get_attachment(uint32_t binding) const
{
    auto it = std::ranges::find_if(attachments, [binding](Attachment const& a) { return a.get_binding() == binding; });
    if (it != attachments.end()) {
        return *it;
    }

    return std::nullopt;
}

Descriptor const* RenderStage::get_descriptor(std::string const& name) const
{
    if (descriptors.find(name) != descriptors.end()) {
        return it->second;
    }
    return nullptr;
}

GLuint const& RenderStage::get_active_framebuffer(uint32_t active_swapchain_image) const
{
    if (active_swapchain_image > framebuffers->get_framebuffers().size()) {
        return framebuffers->get_framebuffers().at(0);
    }

    return framebuffers->get_framebuffers().at(active_swapchain_image);
}
}