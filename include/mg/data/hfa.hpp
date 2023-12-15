#pragma once

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <mg/util/fs.hpp>

namespace mg::data {

struct Hfa {

  // HUNEXGGEFA10 (magic, 12 bytes)
  // Header entry count (4 bytes)
  // Filename (32 * 3 bytes?)
  // Offset from end of header (4 bytes)
  // Size (4 bytes)
  // (padding 8 bytes?)
  // (padding 16 bytes?)

  static constexpr const char *MAGIC = "HUNEXGGEFA10";

  struct __attribute__((packed)) FileHeader {
    // Filename magic
    char magic[12];
    // Number of entries
    uint32_t entry_count;
  };

  struct __attribute__((packed)) PackedEntryHeader {
    // Name of the contained file
    char filename[96];
    // Start offset of the data (LE)
    uint32_t offset;
    // Data size bytes (LE)
    uint32_t size;
    uint8_t _padding[24];
    void to_host_order();
    void to_file_order();
  };
};

struct MappedHfa {
public:
  static std::unique_ptr<MappedHfa>
  parse(std::shared_ptr<mg::fs::MappedFile> backing_data);

  const std::vector<Hfa::PackedEntryHeader> &entries() const {
    return _entries;
  }

  const std::string_view entry_data(int index) const {
    const auto &entry = _entries.at(index);
    const size_t header_size_bytes =
        sizeof(Hfa::FileHeader) +
        sizeof(Hfa::PackedEntryHeader) * _entries.size();
    const size_t offset_bytes = header_size_bytes + entry.offset;
    return std::string_view(
        reinterpret_cast<const char *>(_backing_data->data()) + offset_bytes,
        entry.size);
  }

private:
  MappedHfa(std::shared_ptr<mg::fs::MappedFile> backing_data,
            std::vector<Hfa::PackedEntryHeader> entries)
      : _backing_data(backing_data), _entries(entries) {}

  std::shared_ptr<mg::fs::MappedFile> _backing_data;
  std::vector<Hfa::PackedEntryHeader> _entries;
};

bool hfa_read(const std::string &hed, const std::string &hfa, Hfa &out);
bool hfa_write(const Hfa &in, std::string &hed, std::string &hfa);

} // namespace mg::data

