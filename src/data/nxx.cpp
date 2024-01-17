#include <string.h>

#include <zlib.h>

#include <mg/data/nxx.hpp>
#include <mg/util/endian.hpp>

namespace mg::data {

static const char *MAGIC_NXCX = "NXCX";
static const char *MAGIC_NXGX = "NXGX";

void Nxx::to_host_order() {
  size = mg::le_to_host_u32(size);
  compressed_size = mg::le_to_host_u32(compressed_size);
}

void Nxx::to_file_order() {
  size = mg::host_to_le_u32(size);
  compressed_size = mg::host_to_le_u32(compressed_size);
}

bool is_nxx_data(const std::string_view &in) {
  // Test big enough to have header
  if (in.size() < sizeof(Nxx)) {
    fprintf(stderr, "NXX file too small\n");
    return false;
  }

  // Check for known file magic
  Nxx header = *reinterpret_cast<const Nxx *>(in.data());

  if (!strncmp(header.magic, MAGIC_NXCX, sizeof(header.magic))) {
    return true;
  }

  if (!strncmp(header.magic, MAGIC_NXGX, sizeof(header.magic))) {
    return true;
  }

  return false;
}

bool extract_nxx_header(const std::string_view &in, Nxx &out) {
  // Does this look like nxx?
  if (!is_nxx_data(in)) {
    return false;
  }

  // Pun header
  out = *reinterpret_cast<const Nxx *>(in.data());
  out.to_host_order();
  return true;
}

bool nxx_decompress(const std::string_view &in, std::string &out) {
  // Input large enough to contain header?
  if (in.size() < sizeof(Nxx)) {
    fprintf(stderr, "NXX file too small\n");
    return false;
  }

  // Pun header
  Nxx header = *reinterpret_cast<const Nxx *>(in.data());
  header.to_host_order();

  // Check magic
  const uint8_t *data_ptr = reinterpret_cast<const uint8_t *>(&in[sizeof(Nxx)]);
  if (!strncmp(header.magic, MAGIC_NXCX, sizeof(header.magic))) {
    return nxcx_decompress(header, data_ptr, out);
  } else if (!strncmp(header.magic, MAGIC_NXGX, sizeof(header.magic))) {
    return nxgx_decompress(header, data_ptr, out);
  } else {
    fprintf(stderr, "Invalid file magic\n");
    return false;
  }
}

bool nxgx_decompress(const Nxx &header, const uint8_t *data, std::string &out) {
  // Expand output to hold data
  out.resize(header.size);

  // Create inflate stream
  z_stream istream{};
  istream.avail_in = header.compressed_size;
  istream.next_in = const_cast<uint8_t *>(data);
  istream.avail_out = header.size;
  istream.next_out = reinterpret_cast<uint8_t *>(out.data());
  istream.total_out = 0;

  // Init inflate context
  if (int err = inflateInit2(&istream, 16 + MAX_WBITS) != Z_OK) {
    fprintf(stderr, "zlib error: %d: %s\n", err, istream.msg);
    return false;
  }

  // Perform inflation
  const int err = inflate(&istream, Z_SYNC_FLUSH);
  if (err != Z_OK && err != Z_STREAM_END) {
    fprintf(stderr, "zlib error: %d: %s\n", err, istream.msg);
    return false;
  }

  if (int err = inflateEnd(&istream) != Z_OK) {
    fprintf(stderr, "zlib error: %d: %s\n", err, istream.msg);
    return false;
  }

  return true;
}

bool nxcx_decompress(const Nxx &header, const uint8_t *data, std::string &out) {
  // Expand output to hold data
  out.resize(header.size);

  // Create inflate stream
  z_stream istream{};
  istream.avail_in = header.compressed_size;
  istream.next_in = const_cast<uint8_t *>(data);
  istream.avail_out = header.size;
  istream.next_out = reinterpret_cast<uint8_t *>(out.data());
  istream.total_out = 0;

  // Perform inflation
  inflateInit(&istream);
  const int err = inflate(&istream, Z_FINISH);
  if (err != Z_STREAM_END && err != Z_OK) {
    fprintf(stderr, "zlib error: %d: %s\n", err, istream.msg);
    return false;
  }
  const int err2 = inflateEnd(&istream);
  if (err2 != Z_OK) {
    fprintf(stderr, "zlib error: %d: %s\n", err2, istream.msg);
    return false;
  }

  return true;
}

bool nxgx_compress(const std::string_view &in, std::string &out) {
  // Create stream context
  z_stream dstream{};
  dstream.avail_in = in.size();
  dstream.next_in =
      const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(in.data()));

  // Default output to same size as input + size of header
  out.resize(in.size() + sizeof(Nxx));

  // Set the output start to be past the end of the reserved header area
  dstream.avail_out = out.size();
  dstream.next_out = reinterpret_cast<uint8_t *>(out.data() + sizeof(Nxx));
  dstream.total_out = 0;

  // Init deflate context
  if (int err = deflateInit2(&dstream, -1, 8, 15 + 16, 8, 0) != Z_OK) {
    fprintf(stderr, "zlib init error: %d: %s\n", err, dstream.msg);
    return false;
  }

  // Perform deflate
  int err = deflate(&dstream, Z_FINISH);
  if (err != Z_OK && err != Z_STREAM_END) {
    fprintf(stderr, "zlib deflate error: %d: %s\n", err, dstream.msg);
    return false;
  }

  // Get the final compressed size
  const int compressed_size = dstream.total_out;

  err = deflateEnd(&dstream);
  if (err != Z_OK && err != Z_STREAM_END) {
    fprintf(stderr, "zlib deflateEnd error: %d: %s\n", err, dstream.msg);
    return false;
  }

  // Write the header
  Nxx *header = reinterpret_cast<Nxx *>(out.data());
  header->size = in.size();
  header->compressed_size = compressed_size;
  memcpy(header->magic, MAGIC_NXGX, sizeof(header->magic));
  header->to_file_order();

  // Shrunk output buffer to wrap
  out.resize(sizeof(Nxx) + compressed_size);

  return true;
}

bool nxcx_compress(const std::string_view &in, std::string &out) {
  return false;
}

} // namespace mg::data
