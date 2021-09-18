#include <mg/data/nam.hpp>

namespace mg::data {

bool nam_read(const std::string &data, Nam &out) {
  // If this file isn't aligned to MAX_STRLEN byte entries, it might not be a
  // NAM
  if (data.size() % Nam::MAX_STRLEN != 0) {
    fprintf(stderr, "Wrong size for NAM data file\n");
    return false;
  }

  // Clear output
  out.names.clear();

  // Iterate each record and add
  std::string::size_type read_offset = 0;
  while (read_offset < data.size()) {
    // Read up to nul byte, max strlen or \r\n
    int len = 0;
    const char *str = &data[read_offset];
    for (; len < Nam::MAX_STRLEN && str[len] != '\0' &&
           !(str[len] == '\r' && str[len + 1] == '\n');
         len++) {
    }
    if (len != 0) {
      out.names.emplace_back(str, len);
    }

    read_offset += Nam::MAX_STRLEN;
  }

  return true;
}

} // namespace mg::data
