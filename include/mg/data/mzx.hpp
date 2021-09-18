#pragma once

#include <string>

#include <mg/util/endian.hpp>

namespace mg::data {

struct __attribute__((__packed__)) MzxHeader {
  static constexpr const char *FILE_MAGIC = "MZX0";

  // Maic word
  uint8_t magic[4];
  // LE
  uint32_t decompressed_size;

  // Convert byte order between file order and host order.
  // Care must be taken not to over-convert.
  void to_host_order() {
    decompressed_size = le_to_host_u32(decompressed_size);
  }
  void to_file_order() {
    decompressed_size = host_to_le_u32(decompressed_size);
  }
};

bool mzx_decompress(const std::string &compressed, std::string &out,
                    bool invert = true);

} // namespace mg::data
