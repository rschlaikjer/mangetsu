#include <filesystem>
#include <set>

#include <mg/data/hfa.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

void usage(const char *program_name) {
  fprintf(stderr, "%s input_file [output_dir]\n", program_name);
}

int main(int argc, char **argv) {
  // Parse args
  const char *input_basename = nullptr;
  const char *output_path = nullptr;
  for (int i = 1; i < argc; i++) {
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

  // Check args are OK
  if (input_basename == nullptr) {
    usage(argv[0]);
    return -1;
  }

  // Map the file
  std::shared_ptr<mg::fs::MappedFile> hfa_data =
      mg::fs::MappedFile::open(input_basename);
  if (hfa_data == nullptr) {
    return -1;
  }

  // Parse the data
  auto hfa = mg::data::MappedHfa::parse(hfa_data);
  if (hfa == nullptr) {
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
  auto write_entry = [&](unsigned index) -> int {
    auto entry_data = hfa->entry_data(index);
    std::filesystem::path output_path = output_dir;
    output_path.append(hfa->entries().at(index).filename);
    if (!mg::fs::write_file(output_path.c_str(), entry_data)) {
      return -1;
    }
    fprintf(stderr, "Wrote %lu bytes to %s\n", entry_data.size(),
            output_path.c_str());

    return 0;
  };

  // Iterate the mrg entries and emit
  for (std::vector<std::string>::size_type i = 0; i < hfa->entries().size();
       i++) {
    write_entry(i);
  }

  return 0;
}
