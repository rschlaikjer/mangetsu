#include <mg/data/mrg.hpp>
#include <mg/data/nam.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

#include <filesystem>

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "%s input_basename [out_dir]\n", argv[0]);
    return -1;
  }

  // Test for input files
  const char *input_basename = argv[1];
  const std::string hed_filename = mg::string::format("%s.hed", input_basename);
  const std::string mrg_filename = mg::string::format("%s.mrg", input_basename);

  // Read raw data for hed/nam files
  std::string hed_raw;
  std::string mrg_raw;
  if (!mg::fs::read_file(hed_filename.c_str(), hed_raw)) {
    return -1;
  }
  if (!mg::fs::read_file(mrg_filename.c_str(), mrg_raw)) {
    return -1;
  }

  // Try and read the NAM table as well. If we can't that's OK
  const std::string nam_filename = mg::string::format("%s.nam", input_basename);
  std::string nam_raw;
  mg::data::Nam nam;
  const bool has_nam = mg::fs::read_file(nam_filename.c_str(), nam_raw) &&
                       nam_read(nam_raw, nam);

  // Parse the MRG data
  mg::data::Mrg mrg;
  if (!mg::data::mrg_read(hed_raw, mrg_raw, mrg)) {
    return -1;
  }

  // If we have a NAM and MRG, assert that the filename count matches the entry
  // count
  if (mrg.entries.size() != nam.names.size()) {
    fprintf(stderr,
            "MRG entry count (%lu) does not match NAM entry count (%lu)\n",
            mrg.entries.size(), nam.names.size());
    return -1;
  }

  // Write out each archive entry, either to the current dir or the out dir if
  // specified
  std::filesystem::path output_dir = ".";
  if (argc == 3) {
    output_dir = argv[2];
  }

  // Ensure output dir exists
  if (!std::filesystem::exists(output_dir) &&
      !std::filesystem::create_directories(output_dir)) {
    fprintf(stderr, "Failed to create output path '%s'\n", output_dir.c_str());
    return -1;
  }

  // Iterate the mrg entries and emit
  const std::string output_basename =
      std::filesystem::path(input_basename).stem();
  for (std::vector<std::string>::size_type i = 0; i < mrg.entries.size(); i++) {
    const auto &entry = mrg.entries[i];
    std::string output_filename =
        mg::string::format("%s.%08lu.dat", output_basename.c_str(), i);

    // If we have a name table, use that name as well
    if (has_nam) {
      output_filename = mg::string::format(
          "%s.%08lu.%s.dat", output_basename.c_str(), i, nam.names[i].c_str());
    }

    std::filesystem::path output_path = output_dir;
    output_path.append(output_filename);
    if (!mg::fs::write_file(output_path.c_str(), entry.data)) {
      return -1;
    }
    fprintf(stderr, "Wrote %lu bytes to %s\n", entry.data.size(),
            output_path.c_str());
  }

  return 0;
}
