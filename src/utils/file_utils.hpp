#pragma once

namespace xen {

class FilePath;

namespace FileUtils {

/// Checks if a file is readable (exists and can be opened).
/// \param filepath Path to the file to be checked.
/// \return True if the file is readable, false otherwise.
bool is_readable(FilePath const& filepath);

/// Reads a whole file into a byte array.
/// \param filepath Path to the file to read.
/// \return Content of the file.
std::vector<uint8_t> read_file_to_array(FilePath const& filepath);

/// Reads a whole file into a string.
/// \note The returned string is not trimmed: there can be spaces or empty lines at the beginning or the end if they
/// exist in the file.
/// \param filepath Path to the file to read.
/// \return Content of the file.
std::string read_file_to_string(FilePath const& filepath);
}
}