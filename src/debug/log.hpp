#pragma once

#include "core.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string_view>

namespace xen {
class XEN_API Log {
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

    template <typename... Args>
    static void out(Args const&... args)
    {
        write(args...);
    }

    template <typename... Args>
    static void out(std::string_view const& style, std::string_view const& colour, Args const&... args)
    {
        write(style, colour, args..., Styles::Default);
    }

    template <typename... Args>
    static void vout(std::string_view format, Args const&... args)
    {
        write(std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    static void
    vout(std::string_view const& style, std::string_view const& colour, std::string_view format, Args const&... args)
    {
        write(style, colour, std::vformat(format, std::make_format_args(args...)), Styles::Default);
    }

    template <typename... Args>
    static void debug([[maybe_unused]] Args const&... args)
    {
#ifdef XEN_DEBUG
        out(Styles::Default, Colors::LightBlue, args...);
#endif
    }

    template <typename... Args>
    static void vdebug(std::string_view format, Args const&... args)
    {
#ifdef XEN_DEBUG
        vout(Styles::Default, Colors::LightBlue, format, args...);
#endif
    }

    template <typename... Args>
    static void info(Args const&... args)
    {
        out(Styles::Default, Colors::Green, args...);
    }

    template <typename... Args>
    static void vinfo(std::string_view format, Args const&... args)
    {
        vout(Styles::Default, Colors::Green, format, args...);
    }

    template <typename... Args>
    static void warning(Args const&... args)
    {
        out(Styles::Default, Colors::Yellow, args...);
    }

    template <typename... Args>
    static void vwarning(std::string_view format, Args const&... args)
    {
        vout(Styles::Default, Colors::Yellow, format, args...);
    }

    template <typename... Args>
    static void error(Args const&... args)
    {
        out(Styles::Default, Colors::Red, args...);
    }

    template <typename... Args>
    static void verror(std::string_view format, Args const&... args)
    {
        vout(Styles::Default, Colors::Red, format, args...);
    }

    template <typename... Args>
    static void rt_assert(bool expr, Args const&... args)
    {
        if (!expr) {
            out(Styles::Default, Colors::Magenta, args...);
            assert(false);
        }
    }

    template <typename... Args>
    static void vrt_assert(bool expr, std::string_view format, Args const&... args)
    {
        if (expr) {
            vout(Styles::Default, Colors::Magenta, format, args...);
            assert(false);
        }
    }

    static void open_log(std::filesystem::path const& filepath);
    static void close_log();

private:
    static std::mutex write_mutex;
    static std::ofstream fs;

    template <typename... Args>
    static void write(Args const&... args)
    {
        std::unique_lock<std::mutex> lock(write_mutex);

        ((std::cerr << std::forward<Args const&>(args)), ...);
        if (fs.is_open()) {
            ((fs << std::forward<Args const&>(args)), ...);
        }
        std::cerr << '\n';
    }
};
}