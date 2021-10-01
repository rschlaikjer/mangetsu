#include <string.h>

#include <mg/data/mrg.hpp>
#include <mg/util/endian.hpp>

namespace mg::data {

void Mrg::PackedEntryHeader::to_host_order() {
  offset = mg::le_to_host_u32(offset);
  size_sectors = mg::le_to_host_u16(size_sectors);
  size_uncompressed_sectors = mg::le_to_host_u16(size_uncompressed_sectors);
}

void Mrg::PackedEntryHeader::to_file_order() {
  offset = mg::host_to_le_u32(offset);
  size_sectors = mg::host_to_le_u16(size_sectors);
  size_uncompressed_sectors = mg::host_to_le_u16(size_uncompressed_sectors);
}

bool mrg_read(const std::string &hed, const std::string &mrg, Mrg &out) {
  if (hed.size() % sizeof(Mrg::PackedEntryHeader) != 0) {
    fprintf(stderr, "Wrong size for HED, must be multiple of %lu\n",
            sizeof(Mrg::PackedEntryHeader));
    return false;
  }

  // Reinterpret header
  const ssize_t entry_count = hed.size() / sizeof(Mrg::PackedEntryHeader);
  const Mrg::PackedEntryHeader *raw_entries =
      reinterpret_cast<const Mrg::PackedEntryHeader *>(hed.data());

  // Parse off each entry
  out.entries.clear();
  for (ssize_t i = 0; i < entry_count; i++) {
    // Copy current header
    Mrg::PackedEntryHeader header = raw_entries[i];

    // Endian swap to host
    header.to_host_order();

    // Is this EOF?
    if (header.offset == 0xFFFF'FFFF) {
      break;
    }

    // Excise the source data at the specified offset + len
    out.entries.emplace_back(
        std::string(&mrg[header.offset * Mrg::SECTOR_SIZE],
                    header.size_sectors * Mrg::SECTOR_SIZE));
  }

  return true;
}

bool mrg_write(const Mrg &in, std::string &hed, std::string &mrg) {
  // Work out the total header size
  // Note that there are 2 extra header entries of 0xFF for EOF
  const ssize_t header_size =
      (in.entries.size() + 2) * sizeof(Mrg::PackedEntryHeader);
  hed.resize(header_size);

  // Conversion of real bytes -> total sectors consumed
  auto size_in_sectors = [](unsigned byte_count) -> uint32_t {
    const uint32_t full_sectors = byte_count / Mrg::SECTOR_SIZE;
    return full_sectors + (byte_count % Mrg::SECTOR_SIZE ? 1 : 0);
  };

  // Serialize each entry
  Mrg::PackedEntryHeader *headers =
      reinterpret_cast<Mrg::PackedEntryHeader *>(hed.data());
  ssize_t mrg_write_offset_sectors = 0;
  mrg.clear();
  for (unsigned i = 0; i < in.entries.size(); i++) {
    // Pack header
    headers[i].offset = mrg_write_offset_sectors;
    headers[i].size_sectors = size_in_sectors(in.entries[i].data.size());

    // If the entry is compressed, use the correct uncompressed size
    // Else, use the normal size
    if (in.entries[i].is_compressed) {
      headers[i].size_uncompressed_sectors =
          size_in_sectors(in.entries[i].uncompressed_size_bytes);
    } else {
      headers[i].size_uncompressed_sectors =
          size_in_sectors(in.entries[i].data.size());
    }

    // Copy data
    mrg.resize(mrg.size() + headers[i].size_sectors * Mrg::SECTOR_SIZE, '\0');
    memcpy(&mrg[mrg_write_offset_sectors * Mrg::SECTOR_SIZE],
           in.entries[i].data.data(), in.entries[i].data.size());

    // Increment write offset
    mrg_write_offset_sectors += headers[i].size_sectors;

    // Header to file order
    headers[i].to_file_order();
  }

  // Write EOF on the HED file
  memset(&headers[in.entries.size()], 0xFF, sizeof(Mrg::PackedEntryHeader) * 2);

  return true;
}

} // namespace mg::data
