#include "scene.hpp"

void render::Scene::draw(Frustum const& cam_frustum) {    
    for (Entity& entity : entities) {
        entity.draw_tree(cam_frustum);
    }
}

void render::Scene::update() {
    for (Entity& entity : entities) {
        entity.update_tree();
    }
}