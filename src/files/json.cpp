// #include "json.hpp"
// #include "debug/log.hpp"

// #include <glaze/json.hpp>

// namespace xen {
// template <typename T>
// void JsonFile::read(T& t) noexcept
// {
//     std::string buffer;
//     auto ec = glz::read_file_json(t, file_path, buffer);

//     if (ec) {
//         std::string descriptive_error = glz::format_error(ec, buffer);
//         Log::error("Error while reading json at ", file_path, " with ", descriptive_error);
//     }
// }

// template <typename T>
// void JsonFile::write(T const& t) noexcept
// {
//     std::string buffer;
//     auto ec = glz::write_file_json<glz::opts{.prettify = true}>(t, file_path, buffer);

//     if (ec) {
//         std::string descriptive_error = glz::format_error(ec, buffer);
//         Log::error("Error while writing json at ", file_path, " with ", descriptive_error);
//     }
// }

// template <typename T>
// void JsonFile::write(T&& t) noexcept
// {
//     std::string buffer;
//     auto ec = glz::write_file_json<glz::opts{.prettify = true}>(std::move(t), file_path, buffer);

//     if (ec) {
//         std::string descriptive_error = glz::format_error(ec, buffer);
//         Log::error("Error while writing json at ", file_path, " with ", descriptive_error);
//     }
// }
// }