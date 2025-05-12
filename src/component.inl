namespace xen {
template <typename CompT>
size_t Component::get_id()
{
    static_assert(std::is_base_of_v<Component, CompT>, "Error: The fetched component must be derived from Component.");
    static_assert(
        !std::is_same_v<Component, CompT>, "Error: The fetched component must not be of specific type 'Component'."
    );

    static size_t const id = max_id++;
    return id;
}
}