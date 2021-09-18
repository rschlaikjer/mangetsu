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

  // Split MZP into constituent files
  for (unsigned i = 0; i < mzp.entry_headers.size(); i++) {
    std::string output_filename = mg::string::format("%s_%04u.bin", argv[1], i);
    auto &data = mzp.entry_data[i];
    if (!mg::fs::write_file(output_filename.c_str(), data)) {
      return -1;
    }
    fprintf(stderr, "Wrote %s\n", output_filename.c_str());
  }

  return 0;
}
