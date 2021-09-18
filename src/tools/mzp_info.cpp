#include <mg/data/mzp.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s infile\n", argv[0]);
    return -1;
  }

  // Read input file
  std::string archive_data;
  if (!mg::fs::read_file(argv[1], archive_data)) {
    return -1;
  }

  // Extract header
  mg::data::Mzp mzp;
  if (!mg::data::mzp_read(archive_data, mzp)) {
    fprintf(stderr, "Failed to parse archive\n");
    return -1;
  }

  fprintf(stderr, "MZP archive of %lu elements:\n", mzp.entry_headers.size());
  for (auto &header : mzp.entry_headers) {
    header.print();
  }

  for (unsigned i = 0; i < mzp.entry_headers.size(); i++) {
    for (unsigned j = i + 1; j < mzp.entry_headers.size(); j++) {
      // Do these ranges overlap
      auto &entry_i = mzp.entry_headers[i];
      auto &entry_j = mzp.entry_headers[j];
      const uint32_t entry_i_start = entry_i.data_offset_relative();
      const uint32_t entry_i_end = entry_i_start + entry_i.entry_data_size();
      const uint32_t entry_j_start = entry_j.data_offset_relative();
      const uint32_t entry_j_end = entry_j_start + entry_j.entry_data_size();
      if (entry_j_start >= entry_i_start && entry_j_start <= entry_i_end) {
        fprintf(stderr, "Entry %u begins inside of entry %u\n", j, i);
      }
    }
  }

  return 0;
}
