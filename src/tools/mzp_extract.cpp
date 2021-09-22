#include <mg/data/mzp.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

#include <filesystem>

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "%s infile [out_dir]\n", argv[0]);
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

  // If we are putting output in a specific folder, check it exists
  if (!std::filesystem::exists(argv[2])) {
    fprintf(stderr, "Output directory '%s' does not exist, exiting\n", argv[2]);
    return -1;
  }

  // Helper to get output file path for entries
  auto output_path = [&](unsigned entry) -> std::string {
    std::string file_name = mg::string::format("%s_%04u.bin", argv[1], entry);
    if (argc == 3) {
      return std::filesystem::path(argv[2]).append(file_name);
    } else {
      return std::filesystem::path(file_name);
    }
  };

  // Split MZP into constituent files
  for (unsigned i = 0; i < mzp.entry_headers.size(); i++) {
    std::string path = output_path(i);
    auto &data = mzp.entry_data[i];
    if (!mg::fs::write_file(path.c_str(), data)) {
      return -1;
    }
    fprintf(stderr, "Wrote %s\n", path.c_str());
  }

  return 0;
}
