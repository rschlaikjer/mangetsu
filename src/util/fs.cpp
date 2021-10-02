#include <mg/util/fs.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace mg {
namespace fs {

MappedFile::~MappedFile() { munmap(const_cast<uint8_t *>(_data), _size); }

std::unique_ptr<MappedFile> MappedFile::open(const char *filename) {
  int file_fd = ::open(filename, O_RDONLY);
  if (file_fd < 0) {
    return nullptr;
  }

  std::shared_ptr<void> _defer_close_fd(nullptr,
                                        [=](...) { ::close(file_fd); });

  const off_t file_size = ::lseek(file_fd, 0, SEEK_END);
  if (file_size < 0) {
    return nullptr;
  }
  if (lseek(file_fd, 0, SEEK_SET) < 0) {
    return nullptr;
  }

  void *mmapped_data =
      mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, file_fd, 0);

  if (mmapped_data == nullptr) {
    return nullptr;
  }

  return std::unique_ptr<MappedFile>(
      new MappedFile(reinterpret_cast<uint8_t *>(mmapped_data), file_size));
}

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
  return write_file(path, std::string_view(data));
}

bool write_file(const char *path, const std::string_view &data) {
  // Open file
  const int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
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

} // namespace fs
} // namespace mg
