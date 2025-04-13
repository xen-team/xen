#include "resources.hpp"

#include <condition_variable>
#include <render/texture/cubemap.hpp>
#include <render/mesh/model.hpp>

namespace xen {
Resources::Resources()
{
    auto const thread_count = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() / 2 : 2;

    Log::vinfo(
        "Resources "
        "Spawning {} threads for the asset manager",
        thread_count
    );

    for (uint i = 0; i < thread_count; i++) {
        workers.emplace_back(&Resources::loader_thread, this);
    }
}

Resources::~Resources()
{
    for (auto& worker : workers) {
        worker.join();
    }
}

Model* Resources::load_model(std::string_view path)
{
    Model* model_cached = fetch_model_from_cache(path);
    if (model_cached) {
        return model_cached;
    }

    auto* model = new Model;

    model->load_model(path);
    model->generate_gpu_data();

    auto placed_model = model_cache.emplace(path, model);

    return placed_model.first->second;
}

Model* Resources::load_model_async(std::string_view path, std::function<void(Model*)> callback)
{
    Model* model_cached = fetch_model_from_cache(path);
    if (model_cached) {
        return model_cached;
    }

    auto* model = new Model;

    ModelLoadJob job;
    job.path = path;
    job.model = model;
    if (callback) {
        job.callback = callback;
    }

    auto placed_model = model_cache.emplace(path, model);

    ++assets_in_flight;
    loading_model_queue.push(job);
    loader_cv.notify_one();

    return placed_model.first->second;
}

Model* Resources::fetch_model_from_cache(std::string_view path)
{
    auto iter = model_cache.find(std::string(path));
    if (iter != model_cache.end()) {
        return iter->second;
    }

    return nullptr;
}

// Function force loads a texture on the main thread and blocks until it is generated
Texture* Resources::load_2d_texture(std::string_view path, TextureSettings* settings)
{
    Texture* texture_cached = fetch_texture_from_cache(path);
    if (texture_cached) {
        return texture_cached;
    }

    auto* texture = settings ? new Texture(*settings) : new Texture;

    TextureGenerationData gen_data;
    gen_data.texture = texture;

    TextureLoader::load_2d_texture_data(path, gen_data);
    if (!gen_data.data) {
        delete texture;
        return nullptr;
    }

    TextureLoader::generate_2d_texture(path, gen_data);

    auto placed_texture = texture_cache.emplace(path, texture);

    return placed_texture.first->second;
}

// Function adds the texture to a queue to be loaded by the asset manager's workers threads
Texture* Resources::load_2d_texture_async(
    std::string_view path, TextureSettings* settings, std::function<void(Texture*)> callback
)
{
    Texture* texture_cached = fetch_texture_from_cache(path);
    if (texture_cached) {
        return texture_cached;
    }

    auto* texture = settings ? new Texture(*settings) : new Texture;

    TextureLoadJob job;
    job.texture_path = path;
    job.generation_data.texture = texture;
    if (callback) {
        job.callback = callback;
    }

    auto placed_texture = texture_cache.emplace(path, texture);

    ++assets_in_flight;
    loading_textures_queue.push(job);
    loader_cv.notify_one();

    return placed_texture.first->second;
}

Texture* Resources::fetch_texture_from_cache(std::string_view path)
{
    auto iter = texture_cache.find(std::string(path));
    if (iter != texture_cache.end()) {
        return iter->second;
    }

    return nullptr;
}

Cubemap* Resources::load_cubemap_texture(
    std::string_view right, std::string_view left, std::string_view top, std::string_view bottom, std::string_view back,
    std::string_view front, CubemapSettings* settings
)
{
    auto* cubemap = settings ? new Cubemap(*settings) : new Cubemap;

    constexpr auto faces_count = 6;
    std::array<std::string_view, faces_count> const faces = {right, left, top, bottom, back, front};

    for (int i = 0; i < faces_count; i++) {
        CubemapGenerationData gen_data;
        gen_data.cubemap = cubemap;
        gen_data.face = (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);

        TextureLoader::load_cubemap_texture_data(faces[i], gen_data);
        if (!gen_data.data) {
            delete cubemap;
            return nullptr;
        }

        TextureLoader::generate_cubemap_texture(faces[i], gen_data);
    }

    return cubemap;
}

Cubemap* Resources::load_cubemap_texture_async(
    std::string_view right, std::string_view left, std::string_view top, std::string_view bottom, std::string_view back,
    std::string_view front, CubemapSettings* settings, std::function<void()> callback
)
{
    auto* cubemap = settings ? new Cubemap(*settings) : new Cubemap;

    constexpr auto faces_count = 6;
    std::array<std::string_view, faces_count> const faces = {right, left, top, bottom, back, front};

    for (int i = 0; i < faces_count; i++) {
        CubemapLoadJob job;
        job.texture_path = faces[i];
        job.generation_data.face = (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        job.generation_data.cubemap = cubemap;
        if (callback) {
            job.callback = callback;
        }

        ++assets_in_flight;
        loading_cubemap_queue.push(job);
        loader_cv.notify_one();
    }

    return cubemap;
}

void Resources::loader_thread()
{
    while (async_loading_active) {
        std::unique_lock<std::mutex> lock(loader_mutex);
        if (loading_textures_queue.empty() && loading_cubemap_queue.empty() && loading_model_queue.empty()) {
            loader_cv.wait(lock);
        }

        lock.unlock();

        if (!loading_textures_queue.empty()) {
            TextureLoadJob load_job;
            if (loading_textures_queue.try_pop(load_job)) {
                TextureLoader::load_2d_texture_data(load_job.texture_path, load_job.generation_data);
                generate_textures_queue.push(load_job);
            }
        }
        if (!loading_cubemap_queue.empty()) {
            CubemapLoadJob load_job;
            if (loading_cubemap_queue.try_pop(load_job)) {
                TextureLoader::load_cubemap_texture_data(load_job.texture_path, load_job.generation_data);
                generate_cubemap_queue.push(load_job);
            }
        }
        if (!loading_model_queue.empty()) {
            ModelLoadJob load_job;
            if (loading_model_queue.try_pop(load_job)) {
                load_job.model->load_model(load_job.path);
                generate_model_queue.push(load_job);
            }
        }
    }
}

void Resources::update()
{
    constexpr int textures_per_frame = 2;
    constexpr int cubemap_faces_per_frame = 2;
    constexpr int models_per_frame = 1;

    int textures_processed = 0;
    while (!generate_textures_queue.empty() && textures_processed < textures_per_frame) {
        TextureLoadJob load_job;
        if (generate_textures_queue.try_pop(load_job)) {
            if (!process_texture_load_job(load_job)) {
                break;
            }
            textures_processed++;
        }
    }

    int cubemap_faces_processed = 0;
    while (!generate_cubemap_queue.empty() && cubemap_faces_processed < cubemap_faces_per_frame) {
        CubemapLoadJob load_job;
        if (generate_cubemap_queue.try_pop(load_job)) {
            if (!process_cubemap_load_job(load_job)) {
                break;
            }
            cubemap_faces_processed++;
        }
    }

    int models_processed = 0;
    while (!generate_model_queue.empty() && models_processed < models_per_frame) {
        ModelLoadJob load_job;
        if (generate_model_queue.try_pop(load_job)) {
            if (!process_model_load_job(load_job)) {
                break;
            }
            models_processed++;
        }
    }
}

bool Resources::process_texture_load_job(TextureLoadJob& load_job)
{
    if (!load_job.generation_data.data) {
        texture_cache.erase(load_job.texture_path);
        delete load_job.generation_data.texture;
        --assets_in_flight;
        return false;
    }

    TextureLoader::generate_2d_texture(load_job.texture_path, load_job.generation_data);
    --assets_in_flight;
    if (load_job.callback) {
        load_job.callback(load_job.generation_data.texture);
    }

    return true;
}

bool Resources::process_cubemap_load_job(CubemapLoadJob& load_job)
{
    if (!load_job.generation_data.data) {
        delete load_job.generation_data.data;
        --assets_in_flight;
        return false;
    }

    TextureLoader::generate_cubemap_texture(load_job.texture_path, load_job.generation_data);
    --assets_in_flight;
    if (load_job.callback) {
        load_job.callback();
    }

    return true;
}

bool Resources::process_model_load_job(ModelLoadJob& load_job)
{
    if (load_job.model->meshes.size() == 0) {
        model_cache.erase(load_job.path);
        delete load_job.model;
        --assets_in_flight;
        return false;
    }

    load_job.model->generate_gpu_data();
    --assets_in_flight;
    if (load_job.callback) {
        load_job.callback(load_job.model);
    }

    return true;
}
}