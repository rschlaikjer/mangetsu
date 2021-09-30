#include <mg/data/nxx.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

#include <filesystem>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "%s input output\n", argv[0]);
    return -1;
  }

  // Name args
  const char *input_file = argv[1];
  const char *output_file = argv[2];

  // Read raw input data
  std::string raw;
  if (!mg::fs::read_file(input_file, raw)) {
    return -1;
  }

  // Decompress
  std::string decompressed;
  if (!mg::data::nxx_decompress(raw, decompressed)) {
    fprintf(stderr, "Failed to decompress\n");
    return -1;
  }

  // Write
  if (!mg::fs::write_file(output_file, decompressed)) {
    return -1;
  }

  return 0;
}
