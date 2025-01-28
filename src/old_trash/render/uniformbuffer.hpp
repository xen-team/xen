#include <glad/glad.h>

namespace render {
class UniformBufferObject {
private:
    const char* name = nullptr;
    uint32_t size;

    uint32_t shader_idx{0};
    uint32_t ubo;
public:
    UniformBufferObject(const char* layout_name, uint32_t buffer_size) : name{layout_name}, size{buffer_size} {};

    void bind_shader(uint32_t shader_id);
    void init();

    void update_data(void* data, uint32_t size, uint32_t offset = 0);
};
}