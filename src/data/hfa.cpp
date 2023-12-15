#include <string.h>

#include <mg/data/hfa.hpp>
#include <mg/util/endian.hpp>

namespace mg::data {

void Hfa::PackedEntryHeader::to_host_order() {
  size = mg::le_to_host_u32(size);
  offset = mg::le_to_host_u32(offset);
}

void Hfa::PackedEntryHeader::to_file_order() {
  size = mg::host_to_le_u32(size);
  offset = mg::host_to_le_u32(offset);
}

std::unique_ptr<MappedHfa>
MappedHfa::parse(std::shared_ptr<mg::fs::MappedFile> backing_data) {
  // Check file larger enough to have a header
  if ((size_t)backing_data->size() < sizeof(Hfa::FileHeader)) {
    fprintf(stderr, "File too short to read header\n");
    return nullptr;
  }

  // Check magic is correct
  Hfa::FileHeader header = *(Hfa::FileHeader *)backing_data->data();
  if (!!memcmp(header.magic, Hfa::MAGIC, strlen(Hfa::MAGIC))) {
    fprintf(stderr, "File has invalid magic\n");
    return nullptr;
  }

  // Parse each of the header entries
  std::vector<Hfa::PackedEntryHeader> entries;
  const uint32_t entry_count = le_to_host_u32(header.entry_count);
  const Hfa::PackedEntryHeader *entry_ptr =
      reinterpret_cast<const Hfa::PackedEntryHeader *>(backing_data->data() +
                                                       sizeof(Hfa::FileHeader));
  for (uint32_t i = 0; i < entry_count; i++, entry_ptr++) {
    // Copy the entry, convert it to host order and add it to our entry list
    Hfa::PackedEntryHeader entry = *entry_ptr;
    entry.to_host_order();
    entries.emplace_back(entry);
  }

  return std::unique_ptr<MappedHfa>(new MappedHfa(backing_data, entries));
}

} // namespace mg::data
