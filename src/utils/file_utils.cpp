#include "file_utils.hpp"

#include <utils/filepath.hpp>

#include <tracy/Tracy.hpp>

namespace xen::FileUtils {
namespace {
template <typename T>
T read_file(FilePath const& filepath)
{
    ZoneScopedN("[FileUtils]::read_file");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file) {
        throw std::runtime_error("[FileUtils] Could not open the file '" + filepath + '\'');
    }

    // Note that tellg() does not necessarily return a size, but rather a mark pointing at a specific place in the file
    // When opening a file in binary however, it is currently pretty much always represented as a byte offset
    // See: https://stackoverflow.com/a/22986486/3292304
    auto const file_size = file.tellg();

    if (file_size == -1) {
        throw std::runtime_error("[FileUtils] Failed to get the size of the file '" + filepath + '\'');
    }

    // Returning at the beginning of the file to read it
    file.seekg(0, std::ios::beg);

    T file_content;
    file_content.resize(static_cast<size_t>(file_size));
    file.read(reinterpret_cast<char*>(file_content.data()), static_cast<std::streamsize>(file_size));

    return file_content;
}
}

bool is_readable(FilePath const& filepath)
{
    return std::ifstream(filepath).good();
}

std::vector<unsigned char> read_file_to_array(FilePath const& filepath)
{
    ZoneScopedN("FileUtils::read_file_to_array");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());
    return read_file<std::vector<unsigned char>>(filepath);
}

std::string read_file_to_string(FilePath const& filepath)
{
    ZoneScopedN("FileUtils::read_file_to_string");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());
    return read_file<std::string>(filepath);
}
}