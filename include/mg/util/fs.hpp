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

bool read_file(const char *path, std::string &out) {
  // Open file
  const int fd = open(path, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Failed to open '%s' - %s\n", path, strerror(errno));
    return false;
  }
  std::shared_ptr<void> _defer_close_fd(nullptr, [=](...) { close(fd); });

  static const size_t read_blocksz = 4096;
  char buffer[read_blocksz];
  out.clear();
  ssize_t bytes_read = 0;
  do {
    bytes_read = read(fd, buffer, read_blocksz);
    if (bytes_read == -1) {
      fprintf(stderr, "Failed to read '%s' - %s\n", path, strerror(errno));
      return false;
    }
    out.append(buffer, bytes_read);
  } while (bytes_read == read_blocksz);

  return true;
}

bool write_file(const char *path, const std::string &data) {
  // Open file
  const int fd = open(path, O_RDWR | O_CREAT, 0644);
  if (fd == -1) {
    fprintf(stderr, "Failed to open '%s' - %s\n", path, strerror(errno));
    return false;
  }
  std::shared_ptr<void> _defer_close_fd(nullptr, [=](...) { close(fd); });

  size_t total_bytes_written = 0;
  do {
    int wrote_bytes = write(fd, &data[total_bytes_written],
                            data.size() - total_bytes_written);
    if (wrote_bytes == -1) {
      fprintf(stderr, "Failed to write '%s' - %s\n", path, strerror(errno));
      return false;
    }
    total_bytes_written += wrote_bytes;
  } while (total_bytes_written < data.size());

  return true;
}

} // namespace mg::fs
