namespace xen {
template <typename ShapeT>
ShapeT const& Collider::get_shape() const
{
    static_assert(
        std::is_base_of_v<Shape, ShapeT>, "Error: The fetched collider shape type must be derived from Shape."
    );
    static_assert(
        !std::is_same_v<Shape, ShapeT>, "Error: The fetched collider shape type must not be of specific type 'Shape'."
    );
    Log::rt_assert(dynamic_cast<ShapeT const*>(collider_shape.get()), "Error: Invalid collider shape type.");

    return static_cast<ShapeT const&>(get_shape());
}
}