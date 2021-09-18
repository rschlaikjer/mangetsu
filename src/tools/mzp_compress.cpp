#include <mg/data/mzp.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "%s output input [input..]\n", argv[0]);
    return -1;
  }

  // Constrct new MZP
  mg::data::Mzp mzp;

  // Read each input file as a new MZP record
  for (int i = 2; i < argc; i++) {
    std::string entry_data;
    if (!mg::fs::read_file(argv[i], entry_data)) {
      return -1;
    }

    fprintf(stderr, "Adding file %s\n", argv[i]);
    mzp.entry_headers.emplace_back();
    mzp.entry_data.emplace_back(entry_data);
  }

  // Write out the archive
  std::string mzp_out;
  mg::data::mzp_write(mzp, mzp_out);
  if (!mg::fs::write_file(argv[1], mzp_out)) {
    return -1;
  }
  fprintf(stderr, "Wrote %ld bytes to %s\n", mzp_out.size(), argv[1]);

  return 0;
}
