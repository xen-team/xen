#include "uniformbuffer.hpp"

void render::UniformBufferObject::bind_shader(uint32_t shader_id) {
    uint32_t uniform_bid = glGetUniformBlockIndex(shader_id, "View");
    glUniformBlockBinding(shader_id, uniform_bid, shader_idx++);
}
void render::UniformBufferObject::init() {
    if (!shader_idx) {
        std::cerr << "ERROR::UniformBufferObject::UBO is not initialized correctly because bind_shader isnt used before.\n";
    }

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, size);
}

void render::UniformBufferObject::update_data(void* data, uint32_t size, uint32_t offset) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// // configure a uniform buffer object
// // ---------------------------------
// // first. We get the relevant block indices
// unsigned int uniform_bid_default = glGetUniformBlockIndex(default_shader->ID, "View");
// // then we link each shader's uniform block to this uniform binding point
// glUniformBlockBinding(default_shader->ID, uniform_bid_default, 0);
// // Now actually create the buffer
// glGenBuffers(1, &uboMatrices);
// glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
// glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
// glBindBuffer(GL_UNIFORM_BUFFER, 0);
// // define the range of the buffer that links to a uniform binding point
// glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));