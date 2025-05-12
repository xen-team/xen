#pragma once

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cwctype>
#include <locale>
#include <string>
#include <vector>

namespace xen::StrUtils {
/// Transforms in-place a character to lowercase.
/// \param character Character to be transformed.
/// \return Lowercase character.
inline char to_lower(char& character)
{
    character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return character;
}

/// Transforms in-place a wide character to lowercase.
/// \param character Wide character to be transformed.
/// \return Lowercase wide character.
inline wchar_t to_lower(wchar_t& character)
{
    character = static_cast<wchar_t>(std::towlower(character));
    return character;
}

/// Transforms in-place a string to lowercase.
/// \param text String to be transformed.
/// \return Reference to the lowercase string.
inline std::string& to_lower(std::string& text)
{
    std::ranges::transform(text, text.begin(), [](char character) { return to_lower(character); });
    return text;
}

/// Transforms in-place a wide string to lowercase.
/// \param text Wide string to be transformed.
/// \return Reference to the lowercase wide string.
inline std::wstring& to_lower(std::wstring& text)
{
    std::ranges::transform(text, text.begin(), [](wchar_t character) { return to_lower(character); });
    return text;
}

/// Transforms in-place a character to uppercase.
/// \param character Character to be transformed.
/// \return Uppercase character.
inline char to_upper(char& character)
{
    character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    return character;
}

/// Transforms in-place a wide character to uppercase.
/// \param character Wide character to be transformed.
/// \return Uppercase wide character.
inline wchar_t to_upper(wchar_t& character)
{
    character = static_cast<wchar_t>(std::towupper(character));
    return character;
}

/// Transforms in-place a string to uppercase.
/// \param text String to be transformed.
/// \return Reference to the uppercase string.
inline std::string& to_upper(std::string& text)
{
    std::ranges::transform(text, text.begin(), [](char character) { return to_upper(character); });
    return text;
}

/// Transforms in-place a wide string to uppercase.
/// \param text Wide string to be transformed.
/// \return Reference to the uppercase wide string.
inline std::wstring& to_upper(std::wstring& text)
{
    std::ranges::transform(text, text.begin(), [](wchar_t character) { return to_upper(character); });
    return text;
}

/// Transforms a character to lowercase without modifying the input.
/// \param character Character to be transformed.
/// \return Lowercase character copy.
inline char to_lower_copy(char character)
{
    to_lower(character);
    return character;
}

/// Transforms a wide character to lowercase without modifying the input.
/// \param character Wide character to be transformed.
/// \return Lowercase wide character.
inline wchar_t to_lower_copy(wchar_t character)
{
    to_lower(character);
    return character;
}

/// Transforms a string to lowercase without modifying the input.
/// \param text String to be transformed.
/// \return Lowercase string copy.
inline std::string to_lower_copy(std::string text)
{
    to_lower(text);
    return text;
}

/// Transforms a wide string to lowercase without modifying the input.
/// \param text Wide string to be transformed.
/// \return Lowercase wide string copy.
inline std::wstring to_lower_copy(std::wstring text)
{
    to_lower(text);
    return text;
}

/// Transforms a character to uppercase without modifying the input.
/// \param character Character to be transformed.
/// \return Uppercase character copy.
inline char to_upper_copy(char character)
{
    to_upper(character);
    return character;
}

/// Transforms a wide character to uppercase without modifying the input.
/// \param character Wide character to be transformed.
/// \return Uppercase wide character copy.
inline wchar_t to_upper_copy(wchar_t character)
{
    to_upper(character);
    return character;
}

/// Transforms a string to uppercase without modifying the input.
/// \param text String to be transformed.
/// \return Uppercase string copy.
inline std::string to_upper_copy(std::string text)
{
    to_upper(text);
    return text;
}

/// Transforms a wide string to uppercase without modifying the input.
/// \param text Wide string to be transformed.
/// \return Uppercase wide string copy.
inline std::wstring to_upper_copy(std::wstring text)
{
    to_upper(text);
    return text;
}

/// Removes spaces in-place at the start of a string.
/// \param text String to be trimmed.
/// \return Reference to the trimmed string.
inline std::string& trim_left(std::string& text)
{
    text.erase(text.begin(), std::ranges::find_if_not(text, [](unsigned char c) { return std::isspace(c); }));
    return text;
}

/// Removes spaces in-place at the start of a wide string.
/// \param text Wide string to be trimmed.
/// \return Reference to the trimmed wide string.
inline std::wstring& trim_left(std::wstring& text)
{
    text.erase(text.begin(), std::ranges::find_if_not(text, [](wchar_t c) { return std::iswspace(c); }));
    return text;
}

/// Removes spaces in-place at the end of a string.
/// \param text String to be trimmed.
/// \return Reference to the trimmed string.
inline std::string& trim_right(std::string& text)
{
    text.erase(
        std::find_if_not(text.rbegin(), text.rend(), [](unsigned char c) { return std::isspace(c); }).base(), text.end()
    );
    return text;
}

/// Removes spaces in-place at the end of a wide string.
/// \param text Wide string to be trimmed.
/// \return Reference to the trimmed wide string.
inline std::wstring& trim_right(std::wstring& text)
{
    text.erase(
        std::find_if_not(text.rbegin(), text.rend(), [](wchar_t c) { return std::iswspace(c); }).base(), text.end()
    );
    return text;
}

/// Removes spaces in-place on both ends of a string.
/// \param text String to be trimmed.
/// \return Reference to the trimmed string.
inline std::string& trim(std::string& text)
{
    trim_left(text);
    trim_right(text);
    return text;
}

/// Removes spaces in-place on both ends of a wide string.
/// \param text Wide string to be trimmed.
/// \return Reference to the trimmed wide string.
inline std::wstring& trim(std::wstring& text)
{
    trim_left(text);
    trim_right(text);
    return text;
}

/// Removes spaces at the start of a string without modifying the input.
/// \param text String to be trimmed.
/// \return Trimmed string copy.
inline std::string trim_left_copy(std::string text)
{
    trim_left(text);
    return text;
}

/// Removes spaces at the start of a wide string without modifying the input.
/// \param text Wide string to be trimmed.
/// \return Trimmed wide string copy.
inline std::wstring trim_left_copy(std::wstring text)
{
    trim_left(text);
    return text;
}

/// Removes spaces at the end of a string without modifying the input.
/// \param text String to be trimmed.
/// \return Trimmed string copy.
inline std::string trim_right_copy(std::string text)
{
    trim_right(text);
    return text;
}

/// Removes spaces at the end of a wide string without modifying the input.
/// \param text Wide string to be trimmed.
/// \return Trimmed wide string copy.
inline std::wstring trim_right_copy(std::wstring text)
{
    trim_right(text);
    return text;
}

/// Removes spaces on both ends of a string without modifying the input.
/// \param text String to be trimmed.
/// \return Trimmed string copy.
inline std::string trim_copy(std::string text)
{
    trim(text);
    return text;
}

/// Removes spaces on both ends of a wide string without modifying the input.
/// \param text Wide string to be trimmed.
/// \return Trimmed wide string copy.
inline std::wstring trim_copy(std::wstring text)
{
    trim(text);
    return text;
}

/// Splits a string with a given delimiter.
/// \param text String to be split.
/// \param delimiter Character used to split into parts.
/// \return Parts of the split string.
inline std::vector<std::string> split(std::string text, char delimiter)
{
    trim_right(text);

    std::vector<std::string> parts{};

    while (!text.empty()) {
        size_t const delimiter_pos = text.find_first_of(delimiter);

        if (delimiter_pos > text.size()) {
            parts.emplace_back(std::move(text));
            break;
        }

        parts.emplace_back(text.substr(0, delimiter_pos));
        trim_right(parts.back());

        text.erase(0, delimiter_pos + 1);
        trim_left(text);
    }

    return parts;
}

/// Splits a wide string with a given delimiter.
/// \param text Wide string to be split.
/// \param delimiter Wide character used to split into parts.
/// \return Parts of the split wide string.
inline std::vector<std::wstring> split(std::wstring text, wchar_t delimiter)
{
    trim_right(text);

    std::vector<std::wstring> parts{};

    while (!text.empty()) {
        size_t const delimiter_pos = text.find_first_of(delimiter);

        if (delimiter_pos > text.size()) {
            parts.emplace_back(std::move(text));
            break;
        }

        parts.emplace_back(text.substr(0, delimiter_pos));
        trim_right(parts.back());

        text.erase(0, delimiter_pos + 1);
        trim_left(text);
    }

    return parts;
}

/// Converts a wide string to a UTF-8 encoded one.
/// \param text Wide string to convert.
/// \return Converted UTF-8 string.
inline std::string to_utf8(std::wstring const& text)
{
    PUSH_WARNINGS_STATE
    DISABLE_WARNING_CLANG(-Wdeprecated - declarations)
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(text);
    POP_WARNINGS_STATE
}

/// Returns the current UTF-8 encoded string.
/// \note This does nothing & exists only to avoid using preprocessor switches.
/// \param text UTF-8 string to be returned.
/// \return Input UTF-8 string.
constexpr std::string const& to_utf8(std::string const& text)
{
    return text;
}

/// Converts a UTF-8 encoded string to a wide one.
/// \param text UTF-8 string to convert.
/// \return Converted wide string.
inline std::wstring to_wide(std::string const& text)
{
    PUSH_WARNINGS_STATE
    DISABLE_WARNING_CLANG(-Wdeprecated - declarations)
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(text);
    POP_WARNINGS_STATE
}

/// Returns the current wide string.
/// \note This does nothing & exists only to avoid using preprocessor switches.
/// \param text Wide string to be returned.
/// \return Input wide string.
constexpr std::wstring const& to_wide(std::wstring const& text)
{
    return text;
}
}