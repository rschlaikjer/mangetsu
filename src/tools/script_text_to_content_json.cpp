#include <filesystem>

#include <openssl/sha.h>

#include <json.hpp>

#include <mg/data/mzp.hpp>
#include <mg/util/endian.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

std::string sha256(const std::string &data) {
  uint8_t digest[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const uint8_t *>(data.data()), data.size(), digest);
  return std::string{reinterpret_cast<const char *>(digest),
                     SHA256_DIGEST_LENGTH};
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "%s script_text.mrg out.json\n", argv[0]);
    return -1;
  }

  // Name args
  const char *script_text_filename = argv[1];
  const char *output_filename = argv[2];

  // Try and load script_text
  std::string script_text_raw;
  if (!mg::fs::read_file(script_text_filename, script_text_raw)) {
    fprintf(stderr, "Failed to read script text from '%s'\n",
            script_text_filename);
    return -1;
  }

  // Attempt to parse script text as MZP archive
  mg::data::Mzp mzp;
  if (!mg::data::mzp_read(script_text_raw, mzp)) {
    fprintf(stderr, "Failed to parse script data as MZP\n");
    return -1;
  }

  // The script text consists of 10 entries, consisting of 5 pairs of string
  // table offsets / string table data.
  // All of the string tables other than the first pair seem to just contain a
  // \r\n and nothing else.
  if (mzp.entry_data.size() < 2) {
    fprintf(stderr, "Script data does not contain enough entries\n");
    return -1;
  }

  // Extract the string offset table
  const std::string &string_offsets_raw = mzp.entry_data[0];
  const uint32_t *string_offsets_u32 =
      reinterpret_cast<const uint32_t *>(string_offsets_raw.data());
  const unsigned string_offset_count =
      string_offsets_raw.size() / sizeof(uint32_t);
  std::vector<uint32_t> string_data_offsets;
  string_data_offsets.resize(string_offset_count);
  for (unsigned i = 0; i < string_offset_count; i++) {
    string_data_offsets[i] = mg::be_to_host_u32(string_offsets_u32[i]);
  }

  // Now that we have offsets, iterate the string data and extract from each
  // start offset until we hit \r\n
  const std::string &string_data_raw = mzp.entry_data[1];
  const char *const end_ptr = &string_data_raw[string_data_raw.size() - 1];
  std::vector<std::string> string_data;
  for (uint32_t offset : string_data_offsets) {
    // Get a pointer to the start of string
    const char *const start = &string_data_raw[offset];

    // Seek for \r\n
    const char *seek_r = start;
    const char *seek_n = start + 1;
    bool found_end = false;
    while (seek_n < end_ptr &&
           !(found_end = (*seek_r == '\r' && *seek_n == '\n'))) {
      seek_r++;
      seek_n++;
    }

    // Scoop the data from start to seek_r into a string
    if (found_end) {
      string_data.emplace_back(start, seek_r - start);
    }
  }

  // Generate a content hash for each string and encode as json
  nlohmann::json j;
  for (auto &string : string_data) {
    const std::string digest_bytes = sha256(string);
    const std::string digest_hex = mg::string::bytes_to_hex(digest_bytes);
    j["script_text_by_hash"][digest_hex] = {
        {"jp", string},
        {"en", ""},
        {"notes", ""},
    };
  }

  // Serialize and emit
  const std::string serialized_json = j.dump();
  if (!mg::fs::write_file(output_filename, serialized_json)) {
    fprintf(stderr, "Failed to write '%s'\n", output_filename);
    return -1;
  }

  return 0;
}
