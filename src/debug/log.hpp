#pragma once

#include <cassert>
#include <mutex>
#include <filesystem>
#include <string_view>
#include <iostream>
#include <fstream>

namespace xen {
class Log {
public:
	class Styles {
	public:
		constexpr static std::string_view Default = "\033[0m";
		constexpr static std::string_view Bold = "\033[1m";
		constexpr static std::string_view Dim = "\033[2m";
		constexpr static std::string_view Underlined = "\033[4m";
		constexpr static std::string_view Blink = "\033[5m";
		constexpr static std::string_view Reverse = "\033[7m";
		constexpr static std::string_view Hidden = "\033[8m";
	};

	class Colors {
	public:
		constexpr static std::string_view Default = "\033[39m";
		constexpr static std::string_view Black = "\033[30m";
		constexpr static std::string_view Red = "\033[31m";
		constexpr static std::string_view Green = "\033[32m";
		constexpr static std::string_view Yellow = "\033[33m";
		constexpr static std::string_view Blue = "\033[34m";
		constexpr static std::string_view Magenta = "\033[35m";
		constexpr static std::string_view Cyan = "\033[36m";
		constexpr static std::string_view LightGrey = "\033[37m";
		constexpr static std::string_view DarkGrey = "\033[90m";
		constexpr static std::string_view LightRed = "\033[91m";
		constexpr static std::string_view LightGreen = "\033[92m";
		constexpr static std::string_view LightYellow = "\033[93m";
		constexpr static std::string_view LightBlue = "\033[94m";
		constexpr static std::string_view LightMagenta = "\033[95m";
		constexpr static std::string_view LightCyan = "\033[96m";
		constexpr static std::string_view White = "\033[97m";
	};

	constexpr static std::string_view TimestampFormat = "%H:%M:%S";

	template<typename ... Args>
	static void out(Args ... args) {
		write(args...); 
	}

	template<typename ... Args>
	static void out(const std::string_view& style, const std::string_view& colour, Args ... args) {
		write(style, colour, args..., Styles::Default);
	}

	template<typename ... Args>
	static void debug([[maybe_unused]] Args ... args) {
#ifdef XEN_DEBUG
		out(Styles::Default, Colors::LightBlue, args...);
#endif
	}

	template<typename ... Args>
	static void info(Args ... args) {
		out(Styles::Default, Colors::Green, args...);
	}

	template<typename ... Args>
	static void warning(Args ... args) {
		out(Styles::Default, Colors::Yellow, args...);
	}

	template<typename ... Args>
	static void error(Args ... args) {
		out(Styles::Default, Colors::Red, args...);
	}

	template<typename ... Args>
	static void rt_assert(bool expr, Args ... args) {
		if (expr) {
			out(Styles::Default, Colors::Magenta, args...);
			assert(false);
		}
	}

	static void open_log(const std::filesystem::path &filepath);
	static void close_log();

private:
	static std::mutex write_mutex;
	static std::ofstream fs;

	template<typename ... Args>
	static void write(Args ... args) {
		std::unique_lock<std::mutex> lock(write_mutex);
		
		((std::cerr << std::forward<Args>(args)), ...);
		if (fs.is_open()) {
			((fs << std::forward<Args>(args)), ...);
		}
	}
};
}