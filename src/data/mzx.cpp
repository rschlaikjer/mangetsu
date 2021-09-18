#include <string.h>

#include <mg/data/mzx.hpp>

namespace mg::data {

static const uint8_t CMD_RLE = 0;
static const uint8_t CMD_BACKREF = 1;
static const uint8_t CMD_RINGBUF = 2;
static const uint8_t CMD_LITERAL = 3;

bool mzx_compress(const std::string &raw, std::string &out, bool invert) {
  // Don't bother actually compressing the data for now, just emit a valid
  // stream of literals
  MzxHeader header;
  memcpy(header.magic, MzxHeader::FILE_MAGIC, sizeof(header.magic));
  header.decompressed_size = raw.size();

  // Estimate our final size
  const std::string::size_type output_size =
      sizeof(header) + raw.size() + 2 * ((raw.size() / 128) + 1);
  out.resize(output_size);

  // Write header
  std::string::size_type output_offset = 0;
  header.to_file_order();
  memcpy(out.data(), &header, sizeof(header));
  output_offset += sizeof(header);

  for (std::string::size_type pos = 0; pos < raw.size();) {
    // Len field is 6 bits, each word is 2 bytes, we can write 128 bytes per
    // literal record
    const std::string::size_type bytes_remaining = raw.size() - pos;
    const unsigned bytes_to_write =
        bytes_remaining < 128 ? bytes_remaining : 128;

    // Convert that to a number of words to write.
    // If we have a trailing byte, that needs an extra word.
    uint8_t words_to_write = bytes_to_write / 2;
    if (bytes_to_write & 1) {
      words_to_write++;
    }

    // The number of words to write offset by one in the literal
    words_to_write--;

    // Pack the cmd
    uint8_t cmd = CMD_LITERAL | (words_to_write << 2);
    out[output_offset++] = cmd;

    // Write the next bytes_to_write datums
    memcpy(&out[output_offset], &raw[pos], bytes_to_write);
    if (invert) {
      for (std::string::size_type i = output_offset;
           i < output_offset + bytes_to_write; i++) {
        out[i] ^= 0xFF;
      }
    }
    output_offset += bytes_to_write;

    // Increment pos
    pos += bytes_to_write;
  }

  // Shrink the output size down to the bytes we actually wrote
  out.resize(output_offset);

  return true;
}

bool mzx_decompress(const std::string &compressed, std::string &out,
                    bool invert) {
  // If header is too small, bail immediately
  if (compressed.size() < sizeof(MzxHeader)) {
    fprintf(stderr, "Header too small\n");
    return false;
  }

  // Pun start of data stream into header
  MzxHeader header = *reinterpret_cast<const MzxHeader *>(compressed.data());
  header.to_host_order();

  // If the magic doesn't match, do not try and uncompress
  if (memcmp(header.magic, MzxHeader::FILE_MAGIC, sizeof(header.magic)) != 0) {
    fprintf(stderr, "Invalid file magic\n");
    return false;
  }

  // Maic word
  uint8_t magic[4];

  // Resize output buffer to accomodate decompressed data
  out.resize(header.decompressed_size);

  // Last written short
  uint8_t last[2];
  memset(last, invert ? 0xFF : 0x00, sizeof(last));

  // Ring buffer
  uint16_t ring_buffer[64];
  memset(ring_buffer, invert ? 0xFF : 0x00, sizeof(ring_buffer));

  // Clear counter. Last data is reinitialized on zero.
  int clear_count = 0;

  // Start reading right after the header
  std::string::size_type read_offset = sizeof(MzxHeader);
  std::string::size_type decompress_offset = 0;
  unsigned ring_buffer_write_offset = 0;
  while (read_offset < compressed.size()) {
    // Get type / len
    const uint8_t len_cmd = compressed[read_offset++];
    const unsigned cmd = len_cmd & 0b11;
    const unsigned len = len_cmd >> 2;

    // Reset counter
    if (clear_count <= 0) {
      clear_count = 0x1000;
      memset(last, invert ? 0xFF : 0x00, sizeof(last));
    }

    switch (cmd) {

    case CMD_RLE: {
      // Repeat last two bytes len + 1 times
      for (unsigned i = 0; i <= len; i++) {
        out[decompress_offset++] = last[0];
        out[decompress_offset++] = last[1];
      }
    } break;

    case CMD_BACKREF: {
      const int lookback_distance =
          2 * (static_cast<uint8_t>(compressed[read_offset++]) + 1);
      for (unsigned i = 0; i <= len; i++) {
        const std::string::size_type lookback_offset =
            decompress_offset - lookback_distance;

        // Read 2 bytes into last buffer
        last[0] = out[lookback_offset];
        last[1] = out[lookback_offset + 1];

        // Write those bytes to end of stream
        out[decompress_offset++] = last[0];
        out[decompress_offset++] = last[1];
      }
    } break;

    case CMD_RINGBUF: {
      // Load ring buffer data at position len into last
      *reinterpret_cast<uint16_t *>(last) = ring_buffer[len];

      // Emit last
      out[decompress_offset++] = last[0];
      out[decompress_offset++] = last[1];
    } break;

    case CMD_LITERAL: {
      for (unsigned i = 0; i <= len; i++) {
        const uint8_t r0 = compressed[read_offset++] ^ (invert ? 0xFF : 0x00);
        const uint8_t r1 = compressed[read_offset++] ^ (invert ? 0xFF : 0x00);

        // Emit data
        out[decompress_offset++] = r0;
        out[decompress_offset++] = r1;

        // Update last
        last[0] = r0;
        last[1] = r1;

        // Write to ring buffer
        ring_buffer[ring_buffer_write_offset++] = (r1 << 8) | r0;
        ring_buffer_write_offset &= 0x3f;
      }
      break;
    }
    }
  }

  return true;
}

} // namespace mg::data
