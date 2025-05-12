#include "filepath.hpp"
#include <utils/str_utils.hpp>

namespace {
constexpr size_t recover_last_separator_pos(size_t first_separator_pos, size_t second_separator_pos, size_t invalid_pos)
{
    if (first_separator_pos == invalid_pos) {
        return second_separator_pos;
    }

    // The first separator has been found

    if (second_separator_pos == invalid_pos) {
        return first_separator_pos;
    }

    // Both separators have been found; the max (rightmost) position must be picked

    return std::max(first_separator_pos, second_separator_pos);
}

inline size_t recover_last_separator_pos(std::string const& path_str)
{
    size_t const last_slash_pos = path_str.find_last_of('/');
    size_t const last_backslash_pos = path_str.find_last_of('\\');

    return recover_last_separator_pos(last_slash_pos, last_backslash_pos, std::string::npos);
}

inline size_t recover_last_separator_pos(std::wstring const& path_str)
{
    size_t const last_slash_pos = path_str.find_last_of(L'/');
    size_t const last_backslash_pos = path_str.find_last_of(L'\\');

    return recover_last_separator_pos(last_slash_pos, last_backslash_pos, std::wstring::npos);
}
}

namespace xen {
FilePath::FilePath(char const* path_str)
#if defined(XEN_IS_PLATFORM_WINDOWS) && !defined(XEN_IS_PLATFORM_CYGWIN)
    :
    path{StrUtils::to_wide(path_str)} {}
#else
    : path{path_str}
{
}
#endif

    FilePath::FilePath(const wchar_t* path_str)
#if defined(XEN_IS_PLATFORM_WINDOWS) && !defined(XEN_IS_PLATFORM_CYGWIN)
    :
    path{path_str} {}
#else
    : path{StrUtils::to_utf8(path_str)}
{
}
#endif

    FilePath FilePath::recover_path_to_file(const std::string& path_str)
{
    return path_str.substr(0, recover_last_separator_pos(path_str) + 1);
}

FilePath FilePath::recover_path_to_file(std::wstring const& path_str)
{
    return path_str.substr(0, recover_last_separator_pos(path_str) + 1);
}

FilePath FilePath::recover_filename(std::string const& path_str, bool keep_extension)
{
    std::string filename = path_str.substr(recover_last_separator_pos(path_str) + 1);

    if (!keep_extension) {
        size_t const last_point_pos = filename.find_last_of('.');

        if (last_point_pos != std::string::npos) {
            filename = filename.erase(last_point_pos);
        }
    }

    return filename;
}

FilePath FilePath::recover_filename(std::wstring const& path_str, bool keep_extension)
{
    std::wstring filename = path_str.substr(recover_last_separator_pos(path_str) + 1);

    if (!keep_extension) {
        size_t const last_point_pos = filename.find_last_of(L'.');

        if (last_point_pos != std::wstring::npos) {
            filename = filename.erase(last_point_pos);
        }
    }

    return filename;
}

FilePath FilePath::recover_extension(std::string const& path_str)
{
    size_t const last_point_pos = path_str.find_last_of('.');

    if (last_point_pos == std::string::npos) {
        return FilePath();
    }

    return path_str.substr(path_str.find_last_of('.') + 1);
}

FilePath FilePath::recover_extension(std::wstring const& path_str)
{
    size_t const last_point_pos = path_str.find_last_of(L'.');

    if (last_point_pos == std::wstring::npos) {
        return FilePath();
    }

    return path_str.substr(path_str.find_last_of(L'.') + 1);
}

#if defined(XEN_IS_PLATFORM_WINDOWS) && !defined(XEN_IS_PLATFORM_CYGWIN)
std::string FilePath::to_utf8() const
{
    return StrUtils::to_utf8(path);
}
#else
std::wstring FilePath::to_wide() const
{
    return StrUtils::to_wide(path);
}
#endif

std::ostream& operator<<(std::ostream& stream, const FilePath& filepath)
{
    stream << filepath.to_utf8();
    return stream;
}

std::wostream& operator<<(std::wostream& stream, FilePath const& filepath)
{
    stream << filepath.to_wide();
    return stream;
}
}