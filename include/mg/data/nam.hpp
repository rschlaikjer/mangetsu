#pragma once

#include <stdint.h>

#include <string>
#include <vector>

namespace mg::data {

struct Nam {

  static const int MAX_STRLEN = 32;

  std::vector<std::string> names;
};

bool nam_read(const std::string &data, Nam &out);
bool nam_write(const Nam &in, std::string &out);

} // namespace mg::data
