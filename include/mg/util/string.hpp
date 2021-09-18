#pragma once

#include <stdio.h>

namespace mg::string {

template <typename... Args>
std::string format(const std::string &format, Args... args) {
  size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
  std::string ret;
  ret.resize(size);
  snprintf(&ret[0], size, format.c_str(), args...);
  ret.resize(size - 1);
  return ret;
}

} // namespace mg::string
