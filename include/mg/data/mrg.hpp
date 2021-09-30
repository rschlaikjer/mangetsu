#pragma once

#include <stdint.h>

#include <string>
#include <vector>

namespace mg::data {

struct Mrg {

  static const uint32_t SECTOR_SIZE = 0x800;

  struct __attribute__((packed)) PackedEntryHeader {
    uint32_t offset;
    uint16_t size_sectors;
    uint16_t _padding;

    void to_host_order();
    void to_file_order();
  };

  std::vector<std::string> entries;
};

bool mrg_read(const std::string &hed, const std::string &mrg, Mrg &out);
bool mrg_write(const Mrg &in, std::string &hed, std::string &mrg);

} // namespace mg::data
