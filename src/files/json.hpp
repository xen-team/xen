#pragma once

#include "core.hpp"

#include <filesystem>
#include "debug/log.hpp"
#include <nlohmann/json.hpp>
#include <utility>

namespace xen {
using json = nlohmann::json;

template <typename T>
concept serializable = requires(T a, json j) {
    to_json(j, a);
    from_json(j, a);
};

class XEN_API JsonFile {
private:
    std::filesystem::path file_path;

public:
    JsonFile(std::filesystem::path file_path) : file_path{std::move(file_path)} {}

    void read(serializable auto& data) noexcept
    {
        std::ifstream f(file_path);
        if (!f.is_open()) {
            Log::error("Error while opening file stream at path ", file_path);
            return;
        }

        json in;
        f >> in;

        from_json(in, data);
    }

    void write(serializable auto& data) noexcept
    {
        std::ofstream f(file_path);
        if (!f.is_open()) {
            Log::error("Error while opening file stream at path ", file_path);
            return;
        }

        json out;
        to_json(out, data);
        f << std::setw(4) << out;
    }

    [[nodiscard]] std::filesystem::path get_path() const { return file_path; }
};
}