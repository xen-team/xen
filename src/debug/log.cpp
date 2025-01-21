#include "log.hpp"

namespace xen {
std::mutex Log::write_mutex;
std::ofstream Log::fs;

void Log::open_log(const std::filesystem::path &filepath) {
    std::filesystem::create_directories(filepath.parent_path());
    fs.open(filepath, std::ios::in | std::ios::app);
}

void Log::close_log() {
	fs.close();
}
}