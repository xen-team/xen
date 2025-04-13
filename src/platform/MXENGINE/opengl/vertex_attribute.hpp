#include <cstddef>
#include <cstdint>

namespace xen {
struct VertexAttribute {
    uint32_t type;
    uint16_t components;
    uint16_t entries;
    size_t byte_size;

    template <typename T>
    static VertexAttribute entry();
};
}