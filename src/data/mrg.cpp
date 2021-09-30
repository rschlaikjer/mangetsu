#include <mg/data/mrg.hpp>
#include <mg/util/endian.hpp>

namespace mg::data {

void Mrg::PackedEntryHeader::to_host_order() {
  offset = mg::le_to_host_u32(offset);
  size_sectors = mg::le_to_host_u16(size_sectors);
  _padding = mg::le_to_host_u16(_padding);
}

void Mrg::PackedEntryHeader::to_file_order() {
  offset = mg::host_to_le_u32(offset);
  size_sectors = mg::host_to_le_u16(size_sectors);
  _padding = mg::host_to_le_u16(_padding);
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
    out.entries.emplace_back(&mrg[header.offset * Mrg::SECTOR_SIZE],
                             header.size_sectors * Mrg::SECTOR_SIZE);
  }

  return true;
}

bool mrg_write(const Mrg &in, std::string &hed, std::string &mrg) {
  return false;
}

} // namespace mg::data
