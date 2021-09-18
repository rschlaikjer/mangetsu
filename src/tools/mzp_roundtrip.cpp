#include <mg/data/mzp.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "%s infile outfile\n", argv[0]);
    return -1;
  }

  // Read input file
  std::string archive_data;
  if (!mg::fs::read_file(argv[1], archive_data)) {
    return -1;
  }

  // Parse MZP
  mg::data::Mzp mzp;
  if (!mg::data::mzp_read(archive_data, mzp)) {
    fprintf(stderr, "Failed to parse archive\n");
    return -1;
  }

  // Write it back out
  std::string mzp_out;
  mg::data::mzp_write(mzp, mzp_out);
  if (!mg::fs::write_file(argv[2], mzp_out)) {
    return -1;
  }
  fprintf(stderr, "Wrote %s\n", argv[2]);

  return 0;
}
