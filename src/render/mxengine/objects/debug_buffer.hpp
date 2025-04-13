#include "platform/graphic_api.hpp"

#pragma once

namespace xen {
class AABB;
class BoundingBox;
class BoundingSphere;
class Cone;
class Cylinder;
class Capsule;
class Frustrum;
class Line;
class Rectangle;
class Circle;

class DebugBuffer {
private:
    struct Point {
        Vector3f position;
        Vector4f color;
    };

    using FrontendStorage = std::vector<Point>;

    VertexBufferHandle vbo;
    VertexArrayHandle vao;

    FrontendStorage storage;

    bool draw_as_screen_overlay = false;

public:
    // void Init();
    // void Submit(Line const& line, Vector4 const& color);
    // void Submit(const AABB& box, Vector4 const& color);
    // void Submit(BoundingBox const& box, Vector4 const color);
    // void Submit(BoundingSphere const& sphere, Vector4 const color);
    // void Submit(Cone const& cone, Vector4 const& color);
    // void Submit(Frustrum const& frustrum, Vector4 const& color);
    // void Submit(Cylinder const& cylinder, Vector4 const& color);
    // void Submit(Capsule const& capsule, Vector4 const& color);
    // void Submit(Rectangle const& rectangle, Vector4 const& color);
    // void Submit(Circle const& circle, Vector4 const& color);

    // void ClearBuffer();
    // void SubmitBuffer();
    // size_t GetSize() const;
    // VertexArrayHandle GetVAO() const;
};
}