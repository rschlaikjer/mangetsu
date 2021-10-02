#pragma once

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <string>

namespace mg::fs {

class MappedFile {
public:
  static std::unique_ptr<MappedFile> open(const char *filename);
  ~MappedFile();

public:
  uint8_t *mutable_data();
  const uint8_t *data() const;
  ssize_t size() const;

private:
  MappedFile(const uint8_t *data, ssize_t size) : _data(data), _size(size) {}
  MappedFile(const MappedFile &other) = delete;
  MappedFile &operator=(const MappedFile &other) = delete;

  const uint8_t *const _data;
  const ssize_t _size;
};

bool read_file(const char *path, std::string &out);
bool write_file(const char *path, const std::string &data);

} // namespace mg::fs
