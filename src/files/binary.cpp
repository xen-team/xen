// #include "binary.hpp"
// #include "debug/log.hpp"

// #include <bitsery/bitsery.h>
// #include <bitsery/adapter/stream.h>

// namespace xen {
// template <typename T>
// void BinaryFile::read(T& t) noexcept
// {
//     stream.open(file_path, std::ios::binary | std::ios::in);
//     if (!stream.is_open()) {
//         Log::error("Error while opening file stream at path ", file_path);
//     }

//     auto state = bitsery::quickDeserialization<bitsery::InputStreamAdapter>(stream, t);
//     stream.close();
// }

// template <typename T>
// void BinaryFile::write(T const& t) noexcept
// {
//     stream.open(file_path, std::ios::binary | std::ios::trunc | std::ios::out);
//     if (!stream.is_open()) {
//         Log::error("Error while opening file stream at path ", file_path);
//     }

//     bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> ser{stream};
//     ser.object(t);
//     ser.adapter().flush();
//     stream.close();
// }
// }