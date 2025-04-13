#pragma once

#include "image/image.hpp"

#include <string>

namespace xen {
class CubeMap {
public:
    using CubeMapBindId = int;

private:
    using BindableId = unsigned int;

    std::string filepath;
    size_t width = 0, height = 0, channels = 0;
    BindableId id = 0;
    mutable CubeMapBindId active_id = 0;

public:
    CubeMap();

    CubeMap(CubeMap const&) = delete;
    CubeMap& operator=(CubeMap const&) = delete;
    CubeMap(CubeMap&&) noexcept;
    CubeMap& operator=(CubeMap&&) noexcept;
    ~CubeMap();

    CubeMap(std::string_view filepath);

    void bind() const;

    void unbind() const;

    void bind(CubeMapBindId id) const;

    CubeMapBindId get_bound_id() const;
    
    BindableId get_native_handle() const;

    void load(std::string_view filepath);
    void load(
        std::string_view right, std::string_view left, std::string_view top, std::string_view bottom,
        std::string_view front, std::string_view back
    );

    void load(std::array<Image, 6> const& images);
    void load(std::array<uint8_t*, 6> const& raw_data_rgb, size_t width, size_t height);

    void load_depth(int width, int height);

    void set_max_lod(size_t lod);
    void set_min_lod(size_t lod);

    void generate_mipmaps();

    size_t get_width() const { return width; }
    size_t get_height() const { return height; }
    size_t get_channel_count() const { return channels; }

    std::string_view get_file_path() const { return filepath; }

    void set_internal_engine_tag(std::string_view tag) { filepath = tag; }
    
    bool is_internal_engine_resource() const;

private:
    void free();
};
}