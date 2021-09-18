#include <string.h>

#include <mg/data/mzp.hpp>
#include <mg/util/endian.hpp>

namespace mg::data {

void Mzp::MzpArchiveHeader::to_host_order() {
  archive_entry_count = le_to_host_u16(archive_entry_count);
}
void Mzp::MzpArchiveHeader::to_file_order() {
  archive_entry_count = host_to_le_u16(archive_entry_count);
}

void Mzp::MzpArchiveEntry::to_host_order() {
  sector_offset = le_to_host_u16(sector_offset);
  byte_offset = le_to_host_u16(byte_offset);
  size_sectors = le_to_host_u16(size_sectors);
  size_bytes = le_to_host_u16(size_bytes);
}
void Mzp::MzpArchiveEntry::to_file_order() {
  sector_offset = host_to_le_u16(sector_offset);
  byte_offset = host_to_le_u16(byte_offset);
  size_sectors = host_to_le_u16(size_sectors);
  size_bytes = host_to_le_u16(size_bytes);
}

void Mzp::MzpArchiveEntry::print() {
  fprintf(stderr,
          "MzpArchiveEntry:\n"
          "    Sector offset:   %08x\n"
          "    Byte offset:     %08x\n"
          "    Size sectors:    %08x\n"
          "    Size lowbytes:   %08x\n"
          "    True offset:     %08x\n"
          "    True size:       %08x\n",
          sector_offset, byte_offset, size_sectors, size_bytes,
          (sector_offset * SECTOR_SIZE) + byte_offset, entry_data_size());
}

void Mzp::MzpArchiveEntry::set_offsets(uint32_t offset) {
  sector_offset = offset / SECTOR_SIZE;
  byte_offset = offset % SECTOR_SIZE;
}

void Mzp::MzpArchiveEntry::set_data_size(uint32_t size) {
  // Calculate the number of sectors our data crosses (rounding up)
  const bool crosses_sector = !!(size % SECTOR_SIZE);
  const unsigned sector_bound =
      crosses_sector ? (size / SECTOR_SIZE) + 1 : (size / SECTOR_SIZE);
  size_sectors = sector_bound;
  size_bytes = size & 0xFFFF;
}

void mzp_write(const Mzp &mzp, std::string &out) {
  // Clear output
  out.clear();

  // Write header
  Mzp::MzpArchiveHeader header;
  memcpy(header.magic, Mzp::FILE_MAGIC, sizeof(Mzp::MzpArchiveHeader::magic));
  header.archive_entry_count = mzp.entry_headers.size();
  header.to_file_order();
  out.resize(sizeof(header));
  memcpy(&out[0], &header, sizeof(header));

  // Calculate the data segment start
  std::string::size_type data_segment_start =
      sizeof(Mzp::MzpArchiveHeader) +
      sizeof(Mzp::MzpArchiveEntry) * mzp.entry_headers.size();

  // Clone the header data so that we can recalculate it
  std::vector<Mzp::MzpArchiveEntry> entry_headers = mzp.entry_headers;
  std::string::size_type current_data_offset = 0;
  for (unsigned i = 0; i < mzp.entry_headers.size(); i++) {
    Mzp::MzpArchiveEntry &entry_header = entry_headers[i];
    auto &entry_data = mzp.entry_data[i];
    const std::string::size_type entry_size = entry_data.size();
    entry_header.set_offsets(current_data_offset);
    entry_header.set_data_size(entry_size);
    current_data_offset += entry_size;
    // Pad current data offset to 16 byte boundary
    // const std::string::size_type real_data_end_addr =
    //     data_segment_start + current_data_offset;
    // current_data_offset += 16 - (real_data_end_addr % 16);
  }

  // Calculate total file size
  const std::string::size_type output_size =
      data_segment_start + current_data_offset;
  out.resize(output_size, 0xFF);

  // Write out each datum pair, since we need to use the header to get the
  // correct data offset
  for (unsigned i = 0; i < mzp.entry_headers.size(); i++) {
    // Convert header to file order and emit
    auto &entry_header = entry_headers[i];
    entry_header.to_file_order();
    const std::string::size_type header_output_offset =
        sizeof(Mzp::MzpArchiveHeader) + sizeof(Mzp::MzpArchiveEntry) * i;
    memcpy(&out[header_output_offset], &entry_header, sizeof(entry_header));

    // Insert data at data start + data offset
    auto &entry_data = mzp.entry_data[i];
    memcpy(&out[data_segment_start + entry_header.data_offset_relative()],
           entry_data.data(), entry_data.size());
  }
}

bool mzp_read(const std::string &data, Mzp &out) {
  // Is file large enough to have a header
  if (data.size() < sizeof(Mzp::MzpArchiveHeader)) {
    return false;
  }

  // Read off header
  out.header = *reinterpret_cast<const Mzp::MzpArchiveHeader *>(&data[0]);
  out.header.to_host_order();

  // Valid magic?
  if (memcmp(out.header.magic, Mzp::FILE_MAGIC,
             sizeof(Mzp::MzpArchiveHeader::magic)) != 0) {
    return false;
  }

  // Clear output vecs
  out.entry_headers.clear();
  out.entry_data.clear();

  // Iterate the archive entries
  for (uint16_t i = 0; i < out.header.archive_entry_count; i++) {
    // Calculate start offset of header record
    std::string::size_type archive_header_offset =
        sizeof(Mzp::MzpArchiveHeader) + sizeof(Mzp::MzpArchiveEntry) * i;

    // Clone data
    Mzp::MzpArchiveEntry entry =
        *reinterpret_cast<const Mzp::MzpArchiveEntry *>(
            &data[archive_header_offset]);
    entry.to_host_order();

    // Add to header
    out.entry_headers.emplace_back(entry);
  }

  // Scan through and extract the actual archive data
  for (auto &entry : out.entry_headers) {
    out.entry_data.emplace_back(&data[out.archive_entry_start_offset(entry)],
                                entry.entry_data_size());
  }

  return true;
}

} // namespace mg::data
