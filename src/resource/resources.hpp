#pragma once

#include <condition_variable>
#include <engine/module.hpp>
#include <utils/texture_loader.hpp>
#include <utils/multithreading/lock_free_queue.hpp>
#include <functional>

// Make all pointers shared

namespace xen {
struct TextureSettings;
class Model;

struct TextureLoadJob {
    std::string texture_path;
    TextureGenerationData generation_data;
    std::function<void(Texture*)> callback = nullptr;
};

struct CubemapLoadJob {
    std::string texture_path;
    CubemapGenerationData generation_data;
    std::function<void()> callback = nullptr;
};

struct ModelLoadJob {
    std::string path;
    Model* model = nullptr;
    std::function<void(Model*)> callback = nullptr;
};

class Resources : public Module::Registrar<Resources> {
    inline static bool const registered = Register("Resources", Stage::Pre);

private:
    std::vector<std::thread> workers;
    std::atomic<bool> loading_threads_active;
    bool async_loading_active = true;

    // Keeps tracks of assets in flight, there can be a gap between the two queues and we need a way to know when all
    // in-flight assets are complete. This is only incremented on asset load (main thread) and decremented on main
    // thread when finishing creating the asset
    int assets_in_flight = 0;

    std::unordered_map<std::string, Texture*> texture_cache;
    LockFreeQueue<TextureLoadJob> loading_textures_queue;
    LockFreeQueue<TextureLoadJob> generate_textures_queue;

    LockFreeQueue<CubemapLoadJob> loading_cubemap_queue;
    LockFreeQueue<CubemapLoadJob> generate_cubemap_queue;

    std::unordered_map<std::string, Model*> model_cache;
    LockFreeQueue<ModelLoadJob> loading_model_queue;
    LockFreeQueue<ModelLoadJob> generate_model_queue;

    std::condition_variable loader_cv;
    std::mutex loader_mutex;

public:
    Resources();
    ~Resources();

    void update() override;

    [[nodiscard]] bool is_assets_in_flight() const { return assets_in_flight > 0; }

    Model* load_model(std::string_view path);
    Model* load_model_async(std::string_view path, std::function<void(Model*)> callback = nullptr);

    Texture* load_2d_texture(std::string_view path, TextureSettings* settings = nullptr);
    Texture* load_2d_texture_async(
        std::string_view path, TextureSettings* settings = nullptr, std::function<void(Texture*)> callback = nullptr
    );

    // TODO: HDR loading
    Cubemap* load_cubemap_texture(
        std::string_view right, std::string_view left, std::string_view top, std::string_view bottom,
        std::string_view back, std::string_view front, CubemapSettings* settings = nullptr
    );
    Cubemap* load_cubemap_texture_async(
        std::string_view right, std::string_view left, std::string_view top, std::string_view bottom,
        std::string_view back, std::string_view front, CubemapSettings* settings = nullptr,
        std::function<void()> callback = nullptr
    );

    [[nodiscard]] static Texture* get_white_texture() { return TextureLoader::white_texture; }
    [[nodiscard]] static Texture* get_black_texture() { return TextureLoader::black_texture; }
    [[nodiscard]] static Texture* get_white_srgb_texture() { return TextureLoader::white_texture_srgb; }
    [[nodiscard]] static Texture* get_black_srgb_texture() { return TextureLoader::black_texture_srgb; }
    [[nodiscard]] static Texture* get_default_normal_texture() { return TextureLoader::default_normal; }
    [[nodiscard]] static Texture* get_default_ao_texture() { return TextureLoader::white_texture; }
    [[nodiscard]] static Texture* get_full_metallic_texture() { return TextureLoader::white_texture; }
    [[nodiscard]] static Texture* get_no_metallic_texture() { return TextureLoader::black_texture; }
    [[nodiscard]] static Texture* get_full_roughness_texture() { return TextureLoader::white_texture; }
    [[nodiscard]] static Texture* get_no_roughness_texture() { return TextureLoader::black_texture; }
    [[nodiscard]] static Texture* get_default_water_distortion_texture()
    {
        return TextureLoader::default_water_distortion;
    }

private:
    // Used to load resources asynchronously on a threadpool
    void loader_thread();
    bool process_texture_load_job(TextureLoadJob& load_job);
    bool process_cubemap_load_job(CubemapLoadJob& load_job);
    bool process_model_load_job(ModelLoadJob& load_job);

    Model* fetch_model_from_cache(std::string_view path);
    Texture* fetch_texture_from_cache(std::string_view path);
};
}