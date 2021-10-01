#pragma once

#include <stdint.h>

#include <string>
#include <vector>

namespace mg::data {

struct Mrg {

  static const uint32_t SECTOR_SIZE = 0x800;

  struct __attribute__((packed)) PackedEntryHeader {
    // Start offset in file of data, in sectors.. LE.
    uint32_t offset;
    // Upper bound of data section, in sectors
    uint16_t size_sectors;
    // Upper bound of decomopressed data section, in sectors
    // For data that are not compressed, this is equal to size_sectors.
    uint16_t size_uncompressed_sectors;

    void to_host_order();
    void to_file_order();
  };

  struct Entry {
    Entry(const std::string &data_) : data(data_), is_compressed(false) {}
    Entry(std::string &&data_) : data(data_), is_compressed(false) {}
    Entry(const std::string &data_, bool compressed, uint64_t uncompressed_size)
        : data(data_), is_compressed(compressed),
          uncompressed_size_bytes(uncompressed_size) {}
    Entry(std::string &&data_, bool compressed, uint64_t uncompressed_size)
        : data(data_), is_compressed(compressed),
          uncompressed_size_bytes(uncompressed_size) {}

    // Raw entry data
    std::string data;

    // Is this entry compressed data?
    bool is_compressed;

    // If the entry is compressed, this must be set to the size (in bytes, not
    // sectors) of the uncompressed data
    uint64_t uncompressed_size_bytes;
  };

  std::vector<Entry> entries;
};

bool mrg_read(const std::string &hed, const std::string &mrg, Mrg &out);
bool mrg_write(const Mrg &in, std::string &hed, std::string &mrg);

} // namespace mg::data
