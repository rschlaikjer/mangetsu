#pragma once

#include <openssl/sha.h>

namespace mg::util {

std::string sha256(const std::string &data) {
  uint8_t digest[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const uint8_t *>(data.data()), data.size(), digest);
  return std::string{reinterpret_cast<const char *>(digest),
                     SHA256_DIGEST_LENGTH};
}

} // namespace mg::util
