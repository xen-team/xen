#pragma once

#include <iosfwd>

namespace xen {
class FilePath {
#if defined(_WIN32) && !defined(__CYGWIN__) // Cygwin handles conversions on its own; sticking to UTF-8 strings
    using StringType = std::wstring;
    using CharType = wchar_t;
#else
    using StringType = std::string;
    using CharType = char;
#endif

public:
    FilePath() = default;
    /// Creates a path in a platform-dependent encoding from a given UTF-8 string.
    /// \note On Windows, the given string will be converted to UTF-16.
    /// \param path_str UTF-8 encoded path string.
    FilePath(char const* path_str);

    /// Creates a path in a platform-dependent encoding from a given UTF-8 string.
    /// \note On Windows, the given string will be converted to UTF-16.
    /// \param path_str UTF-8 encoded path string.
    FilePath(std::string const& path_str) : FilePath(path_str.c_str()) {}

    /// Creates a path in a platform-dependent encoding from a given UTF-8 string.
    /// \note On Windows, the given string will be converted to UTF-16.
    /// \param path_str UTF-8 encoded path string.
    FilePath(std::string_view const& path_str) : FilePath(path_str.data()) {}

    /// Creates a path in a platform-dependent encoding from a given wide string.
    /// \note On platforms other than Windows, the given string will be converted to UTF-8. If not using Windows, use
    /// the standard string constructors.
    /// \param path_str Platform-specific encoded wide path string.
    FilePath(wchar_t const* path_str);

    /// Creates a path in a platform-dependent encoding from a given wide string.
    /// \note On platforms other than Windows, the given string will be converted to UTF-8. If not using Windows, use
    /// the standard string constructors.
    /// \param path_str Platform-specific encoded wide path string.
    FilePath(std::wstring const& path_str) : FilePath(path_str.c_str()) {}

    /// Creates a path in a platform-dependent encoding from a given wide string.
    /// \note On platforms other than Windows, the given string will be converted to UTF-8. If not using Windows, use
    /// the standard string constructors.
    /// \param path_str Platform-specific encoded wide path string.
    FilePath(std::wstring_view const& path_str) : FilePath(path_str.data()) {}

    FilePath(FilePath const&) = default;
    FilePath(FilePath&&) noexcept = default;
    FilePath& operator=(FilePath const&) = default;
    FilePath& operator=(FilePath&&) noexcept = default;

    StringType const& get_path() const { return path; }

    CharType const* get_path_str() const { return path.c_str(); }

    bool empty() const { return path.empty(); }

    static FilePath recover_path_to_file(std::string const& path_str);

    static FilePath recover_path_to_file(std::wstring const& path_str);

    static FilePath recover_path_to_file(std::string_view const& path_str)
    {
        return recover_path_to_file(std::string(path_str));
    }

    static FilePath recover_path_to_file(std::wstring_view const& path_str)
    {
        return recover_path_to_file(std::wstring(path_str));
    }

    static FilePath recover_filename(std::string const& path_str, bool keep_extension = true);

    static FilePath recover_filename(std::wstring const& path_str, bool keep_extension = true);

    static FilePath recover_filename(std::string_view const& path_str, bool keep_extension = true)
    {
        return recover_filename(std::string(path_str), keep_extension);
    }

    static FilePath recover_filename(std::wstring_view const& path_str, bool keep_extension = true)
    {
        return recover_filename(std::wstring(path_str), keep_extension);
    }

    static FilePath recover_extension(std::string const& path_str);

    static FilePath recover_extension(std::wstring const& path_str);

    static FilePath recover_extension(std::string_view const& path_str)
    {
        return recover_extension(std::string(path_str));
    }

    static FilePath recover_extension(std::wstring_view const& path_str)
    {
        return recover_extension(std::wstring(path_str));
    }

    FilePath recover_path_to_file() const { return recover_path_to_file(path); }

    FilePath recover_filename(bool keep_extension = true) const { return recover_filename(path, keep_extension); }

    FilePath recover_extension() const { return recover_extension(path); }

#if defined(_WIN32) && !defined(__CYGWIN__)
    std::string to_utf8() const;
    StringType const& to_wide() const { return path; }
#else
    const StringType& to_utf8() const { return path; }
    std::wstring to_wide() const;
#endif

    std::string operator+(char c) const { return to_utf8() + c; }
    std::wstring operator+(wchar_t c) const { return to_wide() + c; }
    std::string operator+(char const* path_str) const { return to_utf8() + path_str; }
    std::wstring operator+(wchar_t const* path_str) const { return to_wide() + path_str; }
    std::string operator+(std::string const& path_str) const { return to_utf8() + path_str; }
    std::wstring operator+(std::wstring const& path_str) const { return to_wide() + path_str; }
    FilePath operator+(FilePath const& filepath) const { return path + filepath.get_path(); }
    bool operator==(char const* path_str) const { return to_utf8() == path_str; }
    bool operator!=(char const* path_str) const { return !(*this == path_str); }
    bool operator==(wchar_t const* path_str) const { return to_wide() == path_str; }
    bool operator!=(wchar_t const* path_str) const { return !(*this == path_str); }
    bool operator==(std::string const& path_str) const { return to_utf8() == path_str; }
    bool operator!=(std::string const& path_str) const { return !(*this == path_str); }
    bool operator==(std::wstring const& path_str) const { return to_wide() == path_str; }
    bool operator!=(std::wstring const& path_str) const { return !(*this == path_str); }
    bool operator==(std::string_view const& path_str) const { return to_utf8() == path_str; }
    bool operator!=(std::string_view const& path_str) const { return !(*this == path_str); }
    bool operator==(std::wstring_view const& path_str) const { return to_wide() == path_str; }
    bool operator!=(std::wstring_view const& path_str) const { return !(*this == path_str); }
    bool operator==(FilePath const& filepath) const { return path == filepath.get_path(); }
    bool operator!=(FilePath const& filepath) const { return !(*this == filepath); }
    explicit operator StringType const&() const { return path; }
    operator CharType const*() const { return path.c_str(); }

private:
    StringType path{};
};

static inline std::string operator+(char c, FilePath const& filepath)
{
    return c + filepath.to_utf8();
}

static inline std::wstring operator+(wchar_t c, FilePath const& filepath)
{
    return c + filepath.to_wide();
}

static inline std::string operator+(char const* path_str, FilePath const& filepath)
{
    return path_str + filepath.to_utf8();
}

static inline std::wstring operator+(wchar_t const* path_str, FilePath const& filepath)
{
    return path_str + filepath.to_wide();
}

static inline std::string operator+(std::string const& path_str, FilePath const& filepath)
{
    return path_str + filepath.to_utf8();
}

static inline std::wstring operator+(std::wstring const& path_str, FilePath const& filepath)
{
    return path_str + filepath.to_wide();
}

extern std::ostream& operator<<(std::ostream& stream, FilePath const& filepath);
extern std::wostream& operator<<(std::wostream& stream, FilePath const& filepath);
}

/// Specialization of std::hash for FilePath.
template <>
struct std::hash<xen::FilePath> {
    /// Computes the hash of the given file path.
    /// \param filepath FilePath to compute the hash of.
    /// \return FilePath's hash value.
    size_t operator()(xen::FilePath const& filepath) const { return std::hash<std::string>()(filepath.to_utf8()); }
};