#include <mg/data/mrg.hpp>
#include <mg/data/nam.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

#include <filesystem>

void usage(const char *program_name) {
  fprintf(stderr, "%s [--csv] input_basename\n", program_name);
}

int main(int argc, char **argv) {
  // Parse args
  bool csv = false;
  const char *input_basename = nullptr;

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--csv")) {
      csv = true;
      continue;
    }
    if (input_basename == nullptr) {
      input_basename = argv[i];
      continue;
    }
    usage(argv[0]);
    return -1;
  }

  if (input_basename == nullptr) {
    usage(argv[0]);
    return -1;
  }

  // Test for input files
  const std::string hed_filename = mg::string::format("%s.hed", input_basename);
  const std::string mrg_filename = mg::string::format("%s.mrg", input_basename);

  // Read raw data for hed files
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
  if (has_nam && mrg->entries().size() != nam.names.size()) {
    fprintf(stderr,
            "MRG entry count (%lu) does not match NAM entry count (%lu)\n",
            mrg->entries().size(), nam.names.size());
    return -1;
  }

  // Print some info
  for (unsigned i = 0; i < mrg->entries().size(); i++) {
    const std::string name_info =
        !has_nam ? ""
                 : mg::string::format(", Name: '%s'", nam.names[i].c_str());
    const bool is_compressed = mrg->entries()[i].size_sectors !=
                               mrg->entries()[i].size_uncompressed_sectors;
    const std::string compress_info =
        !is_compressed
            ? ""
            : mg::string::format(", Uncompressed size 0x%08x sectors",
                                 mrg->entries()[i].size_uncompressed_sectors);
    if (csv) {
      printf("%u,0x%08x,0x%08x,0x%08x,%s\n", i, mrg->entries()[i].offset,
             mrg->entries()[i].size_sectors,
             mrg->entries()[i].size_uncompressed_sectors,
             has_nam ? nam.names[i].c_str() : "");
    } else {
      printf("Entry %8u: Offset 0x%08x, Size 0x%08x sectors%s%s\n", i,
             mrg->entries()[i].offset, mrg->entries()[i].size_sectors,
             compress_info.c_str(), name_info.c_str());
    }
  }

  return 0;
}
