#include <filesystem>
#include <set>

#include <mg/data/mrg.hpp>
#include <mg/data/nam.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

void usage(const char *program_name) {
  fprintf(stderr, "%s [-i index...] input_basename [output_dir]\n",
          program_name);
}

int main(int argc, char **argv) {
  // Parse args
  bool targeted_extract = false;
  std::set<long> extract_indices;

  const char *input_basename = nullptr;
  const char *output_path = nullptr;
  for (int i = 1; i < argc; i++) {
    if (!strcmp("-i", argv[i])) {
      // Check it is followed by an index
      if (i + 1 >= argc) {
        fprintf(stderr, "Missing argument for -i\n");
        return -1;
      }

      // Entry specifier
      targeted_extract = true;
      char *endptr;
      long extract_idx = strtol(argv[i + 1], &endptr, 0);
      if (endptr == argv[i + 1]) {
        fprintf(stderr, "Failed to parse index '%s'\n", argv[i + 1]);
        return -1;
      }

      // Add to extract set
      extract_indices.emplace(extract_idx);

      // Skip arg and loop
      i++;
      continue;
    }

    // Basename?
    if (input_basename == nullptr) {
      input_basename = argv[i];
      continue;
    }

    // Output dir?
    if (output_path == nullptr) {
      output_path = argv[i];
      continue;
    }

    // Invalid state
    usage(argv[0]);
    return -1;
  }

  // Test for input files
  const std::string hed_filename = mg::string::format("%s.hed", input_basename);
  const std::string mrg_filename = mg::string::format("%s.mrg", input_basename);

  // Read raw data for hed file
  std::string hed_raw;
  if (!mg::fs::read_file(hed_filename.c_str(), hed_raw)) {
    return -1;
  }

  // Try and map the mrg data
  std::shared_ptr<mg::fs::MappedFile> mrg_data =
      mg::fs::MappedFile::open(mrg_filename.c_str());
  if (mrg_data == nullptr) {
    return -1;
  }

  // Try and read the NAM table as well. If we can't that's OK
  const std::string nam_filename = mg::string::format("%s.nam", input_basename);
  std::string nam_raw;
  mg::data::Nam nam;
  const bool has_nam = mg::fs::read_file(nam_filename.c_str(), nam_raw) &&
                       nam_read(nam_raw, nam);

  // Parse the MRG data
  auto mrg = mg::data::MappedMrg::parse(hed_raw, mrg_data);
  if (mrg == nullptr) {
    return -1;
  }

  // If we have a NAM and MRG, assert that the filename count matches the entry
  // count
  if (mrg->entries().size() != nam.names.size()) {
    fprintf(stderr,
            "MRG entry count (%lu) does not match NAM entry count (%lu)\n",
            mrg->entries().size(), nam.names.size());
    return -1;
  }

  // Ensure output dir exists
  std::filesystem::path output_dir = output_path ? output_path : ".";
  if (!std::filesystem::exists(output_dir) &&
      !std::filesystem::create_directories(output_dir)) {
    fprintf(stderr, "Failed to create output path '%s'\n", output_dir.c_str());
    return -1;
  }

  // Utility method to write an index to an output file
  const std::string output_basename =
      std::filesystem::path(input_basename).stem();
  auto write_entry = [&](unsigned index) -> int {
    std::string output_filename =
        mg::string::format("%s.%08lu.dat", output_basename.c_str(), index);

    // If we have a name table, use that name as well
    if (has_nam) {
      output_filename =
          mg::string::format("%s.%08lu.%s.dat", output_basename.c_str(), index,
                             nam.names[index].c_str());
    }

    auto entry_data = mrg->entry_data(index);
    std::filesystem::path output_path = output_dir;
    output_path.append(output_filename);
    if (!mg::fs::write_file(output_path.c_str(), entry_data)) {
      return -1;
    }
    fprintf(stderr, "Wrote %lu bytes to %s\n", entry_data.size(),
            output_path.c_str());

    return 0;
  };

  // Iterate the mrg entries and emit
  if (!targeted_extract) {
    for (std::vector<std::string>::size_type i = 0; i < mrg->entries().size();
         i++) {
      write_entry(i);
    }
  } else {
    for (long i : extract_indices) {
      write_entry(i);
    }
  }

  return 0;
}
