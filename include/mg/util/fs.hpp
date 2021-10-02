#pragma once

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <string_view>

namespace mg::fs {

class MappedFile {
public:
  static std::unique_ptr<MappedFile> open(const char *filename);
  ~MappedFile();

public:
  const uint8_t *data() const { return _data; }
  ssize_t size() const { return _size; }
  std::string_view string_view() const {
    return std::string_view(reinterpret_cast<const char *>(_data), _size);
  }

private:
  MappedFile(const uint8_t *data, ssize_t size) : _data(data), _size(size) {}
  MappedFile(const MappedFile &other) = delete;
  MappedFile &operator=(const MappedFile &other) = delete;

  const uint8_t *const _data;
  const ssize_t _size;
};

bool read_file(const char *path, std::string &out);
bool write_file(const char *path, const std::string_view &data);
bool write_file(const char *path, const std::string &data);

} // namespace mg::fs
