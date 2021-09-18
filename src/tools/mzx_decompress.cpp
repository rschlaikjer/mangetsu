#include <mg/data/mzx.hpp>
#include <mg/util/fs.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "%s infile outfile\n", argv[0]);
    return -1;
  }

  // Read input file
  std::string compressed;
  if (!mg::fs::read_file(argv[1], compressed)) {
    return -1;
  }

  // Decompress
  std::string decompressed;
  if (!mg::data::mzx_decompress(compressed, decompressed)) {
    fprintf(stderr, "Decompress failed\n");
    return -1;
  }

  // Emit
  if (!mg::fs::write_file(argv[2], decompressed)) {
    return -1;
  }

  return 0;
}
