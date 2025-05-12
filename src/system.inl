namespace xen {
template <typename SysT>
size_t System::get_id()
{
    static_assert(std::is_base_of_v<System, SysT>, "Error: The fetched system must be derived from System.");
    static_assert(!std::is_same_v<System, SysT>, "Error: The fetched system must not be of specific type 'System'.");

    static size_t const id = max_id++;
    return id;
}
}