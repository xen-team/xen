#pragma once

#include "camera.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "uniformbuffer.hpp"
#include "utils/units.hpp"

#include <memory>
#include <glad/glad.h>

namespace render {

    class RenderManager {
    private:
        PerspectiveCamera debug_camera;
        Scene default_scene;

        std::unique_ptr<UniformBufferObject> ubo;
        std::unique_ptr<Shader> default_shader;
    public:
        unsigned int uboMatrices;
        RenderManager();

        static RenderManager& get_manager() {
            static RenderManager single_manager;
            return single_manager;
        }

        void draw_debug();
        void draw(Scene& scene, PerspectiveCamera& from_camera);               

        Scene& get_default_scene() { return default_scene; };
        PerspectiveCamera& get_debug_cam() { return debug_camera; };
        Shader* get_default_shader() { return default_shader.get(); };
    };

    static void clear_frame(br_color_normal clear_col, GLbitfield flags) {
        glClearColor(clear_col.r, clear_col.g, clear_col.b, clear_col.a);
        glClear(flags);
    }

    static void resize_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(0, 0, width, height);
    }

    static void resize_viewport(uint32_t x, uint32_t y, br_size size) {
        glViewport(0, 0, size.w, size.h);
    }
}