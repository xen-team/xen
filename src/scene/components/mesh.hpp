#pragma once
#include <render/mesh/model.hpp>
#include <scene/component.hpp>
#include <utility>

namespace xen {
struct MeshComponent : public Component {
    inline static bool const registered = register_component<MeshComponent>();

    Model model;

    bool is_transparent = false;      // Should be true if the model contains any translucent material
    bool is_static = false;           // Should be true if the model will never have its transform modified
    bool should_backface_cull = true; // Should be true for majority of models, unless a model isn't double sided

    MeshComponent() = default;
    MeshComponent(MeshComponent const& other) = default;
    MeshComponent(Model other_model) : model(std::move(other_model)) {}

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(model, is_transparent, is_static, should_backface_cull);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(model, is_transparent, is_static, should_backface_cull);
    }
};
}