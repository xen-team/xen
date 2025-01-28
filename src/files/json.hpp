#pragma once

#include "core.hpp"

#include <filesystem>
#include "debug/log.hpp"
#include <json.hpp>
#include <utility>

namespace xen {
using json = nlohmann::json;

template <typename T>
concept serializable = requires(T a, nlohmann::json j) {
    a.load(j);
    a.save(j);
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
        data.load(in);
    }

    void write(serializable auto& data) noexcept
    {
        std::ofstream f(file_path);
        if (!f.is_open()) {
            Log::error("Error while opening file stream at path ", file_path);
            return;
        }

        json out;
        data.save(out);
        f << std::setw(4) << out;
    }

    std::filesystem::path get_path() const { return file_path; }
};
}