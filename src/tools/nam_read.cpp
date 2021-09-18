#include <mg/data/nam.hpp>
#include <mg/util/fs.hpp>

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

  // Parse nam
  mg::data::Nam nam;
  if (!mg::data::nam_read(archive_data, nam)) {
    fprintf(stderr, "Failed to parse file\n");
    return -1;
  }

  for (auto &name : nam.names) {
    printf("%s\n", name.c_str());
  }

  return 0;
}
