#pragma once

#include <stdint.h>

#include <string>
#include <vector>

namespace mg::data {

struct Mzp {

  // Archive file format
  // All integer fields are little-endian

  static const uint32_t SECTOR_SIZE = 0x800;
  static constexpr const char *FILE_MAGIC = "mrgd00";

  struct __attribute__((__packed__)) MzpArchiveHeader {
    uint8_t magic[6];
    uint16_t archive_entry_count;
    void to_host_order();
    void to_file_order();
  };
  static_assert(sizeof(MzpArchiveHeader) == 8);

  struct __attribute__((__packed__)) MzpArchiveEntry {
    // Start location of this entry in whole sectors
    uint16_t sector_offset;
    // Start location of this entry (sub-sector positioning)
    uint16_t byte_offset;
    // The upper bound on the number of sectors that this data exists on.
    uint16_t size_sectors;
    // The raw low 16 bits of the full size of the archive data
    uint16_t size_bytes;

    void to_host_order();
    void to_file_order();

    // Update the sector/byte offsets based on an absolute offset
    void set_offsets(uint32_t offset);

    // Update the data size sectors/bytes based on absolute size
    void set_data_size(uint32_t size);

    void print();

    uint32_t entry_data_size() const {
      // The `size` field is the raw 16 low 16 bits of the full size of the
      // entry in bytes. To get our total size, calculate the total number of
      // 2^16 bit pages that are part of this entry, then add on the least
      // significant bits
      const uint32_t upper_bound_size_bytes = size_sectors * SECTOR_SIZE;
      const uint32_t size = (upper_bound_size_bytes & ~(0xFFFF)) | size_bytes;
      return size;
    }

    uint32_t data_offset_relative() const {
      return sector_offset * SECTOR_SIZE + byte_offset;
    }
  };
  static_assert(sizeof(MzpArchiveEntry) == 8);

  MzpArchiveHeader header;
  std::vector<MzpArchiveEntry> entry_headers;
  std::vector<std::string> entry_data;

  uint32_t data_start_offset() {
    return sizeof(MzpArchiveHeader) +
           entry_headers.size() * sizeof(MzpArchiveEntry);
  }

  uint32_t archive_entry_start_offset(const MzpArchiveEntry &entry) {
    return data_start_offset() + entry.data_offset_relative();
  }
};

bool mzp_read(const std::string &data, Mzp &out);
void mzp_write(const Mzp &mzp, std::string &out);

} // namespace mg::data
