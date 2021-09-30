#include <mg/data/mrg.hpp>
#include <mg/data/nam.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

#include <filesystem>
#include <sstream>
#include <string>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "%s output_basename [--names name_list] inputs...\n",
            argv[0]);
    return -1;
  }

  // Parse args
  const char *output_basename = argv[1];
  const char *names_file = nullptr;
  std::vector<const char *> inputs;
  for (int i = 2; i < argc; i++) {
    // Is this a names flag?
    if (!strcmp("--names", argv[i])) {
      // Do we have the next arg?
      if (i + 1 >= argc) {
        fprintf(stderr, "Missing argument for --names\n");
        return -1;
      }
      names_file = argv[i + 1];
      // Advance arg pointer past filename
      i++;
      continue;
    }

    // If it's not a flag, add it to the list of input files
    inputs.emplace_back(argv[i]);
  }

  // Read in each source file to a MRG entry
  mg::data::Mrg mrg;
  for (const char *input : inputs) {
    std::string data;
    if (!mg::fs::read_file(input, data)) {
      return -1;
    }
    mrg.entries.emplace_back(data);
  }

  // If we were given a name file, create a NAM file as well
  mg::data::Nam nam;
  if (names_file != nullptr) {
    std::string name_data;
    if (!mg::fs::read_file(names_file, name_data)) {
      return -1;
    }

    // Split the name input file on newline to get a list of names
    std::stringstream ss(name_data);
    std::string line;
    while (std::getline(ss, line, '\n')) {
      nam.names.emplace_back(line);
    }
  }

  // Serialize mrg/hed
  std::string mrg_out, hed_out;
  if (!mg::data::mrg_write(mrg, hed_out, mrg_out)) {
    fprintf(stderr, "Failed to pack MRG\n");
    return -1;
  }

  // Write outputs
  std::string hed_filename = mg::string::format("%s.hed", output_basename);
  std::string mrg_filename = mg::string::format("%s.mrg", output_basename);
  if (!mg::fs::write_file(hed_filename.c_str(), hed_out)) {
    return -1;
  }
  if (!mg::fs::write_file(mrg_filename.c_str(), mrg_out)) {
    return -1;
  }

  // Serialize nam if given
  std::string nam_out;
  if (nam.names.size() != 0) {
    if (!mg::data::nam_write(nam, nam_out)) {
      fprintf(stderr, "Failed to serialize NAM\n");
      return -1;
    }
    std::string nam_filename = mg::string::format("%s.nam", output_basename);
    if (!mg::fs::write_file(nam_filename.c_str(), nam_out)) {
      return -1;
    }
  }

  return 0;
}
