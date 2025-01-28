#pragma once

#include "core.hpp"

#include <filesystem>
#include <fstream>
#include <utility>

namespace xen {
class XEN_API BinaryFile {
private:
    std::filesystem::path file_path;
    std::fstream stream;

public:
    BinaryFile(std::filesystem::path file_path) : file_path{std::move(file_path)} {}

    template <typename T>
    void read(T& t) noexcept;

    template <typename T>
    void write(T const& t) noexcept;

    std::filesystem::path get_path() const { return file_path; }
};
}

// class XEN_API Binary {
// private:
//     using Buffer = std::vector<uint8_t>;
//     using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
//     using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

//     std::filesystem::path file_path;
//     Buffer buffer;
//     size_t buffer_size;

// private:
//     void read();
//     void write();
// };
