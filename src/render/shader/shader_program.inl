namespace xen {
template <typename T>
bool ShaderProgram::has_attribute(std::string const& uniform_name) const
{
    return (has_attribute(uniform_name) && std::holds_alternative<T>(attributes.find(uniform_name)->second.value));
}

template <typename T>
T const& ShaderProgram::get_attribute(std::string const& uniform_name) const
{
    Log::rt_assert(has_attribute(uniform_name), "Error: The given attribute uniform name does not exist.");
    Log::rt_assert(has_attribute<T>(uniform_name), "Error: The fetched attribute is not of the asked type.");

    return std::get<T>(attributes.find(uniform_name)->second.value);
}

template <typename T>
void ShaderProgram::set_attribute(T&& attrib_val, std::string const& uniform_name)
{
    auto const attr_it = attributes.find(uniform_name);

    if (attr_it != attributes.end()) {
        attr_it->second.value = std::forward<T>(attrib_val);
    }
    else {
        int const location_index = (is_linked() ? recover_uniform_location(uniform_name) : -1);
        attributes.emplace(uniform_name, Attribute{location_index, std::forward<T>(attrib_val)});
    }
}
}