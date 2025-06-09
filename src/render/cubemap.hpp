#pragma once

#include <data/owner_value.hpp>

namespace xen {

class Image;
class RenderShaderProgram;

class Cubemap {
public:
    Cubemap();
    explicit Cubemap(
        Image const& right, Image const& left, Image const& top, Image const& bottom, Image const& front,
        Image const& back
    ) : Cubemap()
    {
        load(right, left, top, bottom, front, back);
    }
    Cubemap(Cubemap const&) = delete;
    Cubemap(Cubemap&&) noexcept = default;

    Cubemap& operator=(Cubemap const&) = delete;
    Cubemap& operator=(Cubemap&&) noexcept = default;

    ~Cubemap();

    [[nodiscard]] uint32_t get_index() const { return index; }

    [[nodiscard]] RenderShaderProgram const& get_program() const;

    /// Applies the given images to the cubemap.
    /// \param right Image which will be on the right of the cube.
    /// \param left Image which will be on the left of the cube.
    /// \param top Image which will be on the top of the cube.
    /// \param bottom Image which will be on the bottom of the cube.
    /// \param front Image which will be on the front of the cube.
    /// \param back Image which will be on the back of the cube.
    void load(
        Image const& right, Image const& left, Image const& top, Image const& bottom, Image const& front,
        Image const& back
    ) const;

    /// Binds the cubemap texture.
    void bind() const;

    /// Unbinds the cubemap texture.
    void unbind() const;

    /// Draws the cubemap around the scene.
    void draw() const;

private:
    OwnerValue<uint32_t> index;
};
}