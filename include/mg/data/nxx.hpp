#pragma once

#include <stdint.h>

#include <string>
#include <string_view>
#include <vector>

namespace mg::data {

// Handler for both NXGX and NXCX file formats
struct __attribute__((packed)) Nxx {
  char magic[4];
  uint32_t size;
  uint32_t compressed_size;
  uint32_t _padding = 0;

  void to_host_order();
  void to_file_order();
};
static_assert(sizeof(Nxx) == 16);

bool is_nxx_data(const std::string_view &data);
bool extract_nxx_header(const std::string_view &data, Nxx &out);

bool nxx_decompress(const std::string_view &in, std::string &out);
bool nxgx_decompress(const Nxx &header, const uint8_t *data, std::string &out);
bool nxcx_decompress(const Nxx &header, const uint8_t *data, std::string &out);

bool nxgx_compress(const std::string_view &in, std::string &out);
bool nxcx_compress(const std::string_view &in, std::string &out);

} // namespace mg::data
