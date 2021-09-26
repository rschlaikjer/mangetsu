#include <filesystem>

#include <json.hpp>

#include <mg.hpp>
#include <mg/data/mzp.hpp>
#include <mg/util/crypto.hpp>
#include <mg/util/endian.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

static const std::string TARGET_LANGUAGE = "en";

std::vector<uint32_t>
parse_offset_table(const std::string &string_offsets_raw) {
  // Extract the string offset table
  const uint32_t *string_offsets_u32 =
      reinterpret_cast<const uint32_t *>(string_offsets_raw.data());
  const unsigned string_offset_count =
      string_offsets_raw.size() / sizeof(uint32_t);
  std::vector<uint32_t> string_data_offsets;
  string_data_offsets.resize(string_offset_count);
  for (unsigned i = 0; i < string_offset_count; i++) {
    string_data_offsets[i] = mg::be_to_host_u32(string_offsets_u32[i]);
  }
  return string_data_offsets;
}

std::vector<std::string>
extract_string_table(const std::string &string_data_raw,
                     const std::vector<uint32_t> &string_offsets) {
  // Now that we have offsets, iterate the string data and extract from each
  // start offset until we hit \r\n
  const char *const end_ptr = &string_data_raw[string_data_raw.size() - 1];
  std::vector<std::string> string_data;
  for (uint32_t offset : string_offsets) {
    // Have we hit EOF?
    if (offset == 0xFFFFFFFF) {
      break;
    }

    // Get a pointer to the start of string
    const char *const start = &string_data_raw[offset];

    // Seek for \r\n
    const char *seek_r = start;
    const char *seek_n = start + 1;
    bool found_end = false;
    while (seek_n <= end_ptr &&
           !(found_end = (*seek_r == '\r' && *seek_n == '\n'))) {
      seek_r++;
      seek_n++;
    }

    // Scoop the data from start to seek_r into a string
    if (found_end) {
      string_data.emplace_back(start, seek_r - start);
    } else {
      fprintf(stderr, "Warning - failed to load string at offset %08x\n",
              offset);
    }
  }

  return string_data;
}

mg::data::Mzp patch_string_table(const nlohmann::json &translation_db,
                                 const mg::data::Mzp mzp_archive) {
  // The MZP archive consists of N pairs of string offset table + string data
  // table. For each pair, iterate the string table, extract the string, hash
  // it, check if the translated string exists, and if so inject the translated
  // string instead.
  ASSERT(mzp_archive.entry_headers.size() == mzp_archive.entry_data.size());
  ASSERT(mzp_archive.entry_headers.size() % 2 == 0);

  // Return MZP
  mg::data::Mzp ret;

  for (unsigned i = 0; i < mzp_archive.entry_headers.size(); i += 2) {
    const unsigned offset_table_idx = i;
    const unsigned string_table_idx = i + 1;

    // Extract original strings
    const std::vector<uint32_t> offsets =
        parse_offset_table(mzp_archive.entry_data[offset_table_idx]);
    const std::vector<std::string> original_strings =
        extract_string_table(mzp_archive.entry_data[string_table_idx], offsets);
    fprintf(stderr, "Loaded %lu strings from tables %u+%u\n",
            original_strings.size(), offset_table_idx, string_table_idx);

    // Iterate the strings, and replace any translated ones with the appropriate
    // text
    std::vector<std::string> translated_strings;
    for (auto &line : original_strings) {
      // Hash the line
      const std::string line_digest_hex =
          mg::string::bytes_to_hex(mg::util::sha256(line));

      // If we find nothing, fall back to the original text
      std::string line_to_insert = line;

      // Is this hash in our TL DB?
      auto &script_text_by_hash = translation_db["script_text_by_hash"];
      if (script_text_by_hash.contains(line_digest_hex)) {
        if (script_text_by_hash[line_digest_hex].contains(TARGET_LANGUAGE)) {
          const std::string translated_line =
              script_text_by_hash[line_digest_hex][TARGET_LANGUAGE];
          if (translated_line.size() > 0) {
            // We have a translation
            line_to_insert = translated_line;
          }
        }
      }

      // Append the line to our output vector
      translated_strings.emplace_back(line_to_insert);
    }

    // Now that we have a vector of translated strings, we need to insert them
    // back into the MZP and update the offset table.
    // Note that during reinsertion, we re-add the \r\n escape sequence at the
    // end of each line
    std::vector<uint32_t> new_text_offsets;
    std::string new_text_data;
    uint32_t text_write_offset = 0;
    for (auto &line : translated_strings) {
      // Append \r\n
      const std::string padded_line = line + "\r\n";

      // Append this string to our buffer
      new_text_data.resize(new_text_data.size() + padded_line.size());
      memcpy(&new_text_data[text_write_offset], padded_line.data(),
             padded_line.size());

      // Add that offset for this string to the offset table
      new_text_offsets.emplace_back(text_write_offset);

      // Increment the write pointer by the size of the string
      text_write_offset += padded_line.size();
    }

    // For some reason, the offsets seem to include 2 instances of offsets that
    // point to the final byte of the string table. Recreate them here in case
    // the game actually needs this info.
    new_text_offsets.emplace_back(new_text_data.size());
    new_text_offsets.emplace_back(new_text_data.size());

    // The offset table ends with 12 bytes of 0xFF
    new_text_offsets.emplace_back(0xFFFFFFFF);
    new_text_offsets.emplace_back(0xFFFFFFFF);
    new_text_offsets.emplace_back(0xFFFFFFFF);

    // Now that we have the new string data, and the new list of offsets, pack
    // the list of offsets back into a byte array
    std::string serialized_new_offsets;
    serialized_new_offsets.resize(new_text_offsets.size() * sizeof(uint32_t));
    uint32_t offset_write_offset = 0;
    for (uint32_t offset : new_text_offsets) {
      const uint32_t be_offset = mg::host_to_be_u32(offset);
      memcpy(&serialized_new_offsets[offset_write_offset], &be_offset,
             sizeof(be_offset));
      offset_write_offset += sizeof(uint32_t);
    }

    // Add the new segments to our output MZP
    ret.add_entry(serialized_new_offsets);
    ret.add_entry(new_text_data);
  }

  return ret;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "%s translation_db.json script_in.mrg script_out.mrg\n",
            argv[0]);
    return -1;
  }

  // Name args
  const char *translation_db_filename = argv[1];
  const char *input_mrg = argv[2];
  const char *output_mrg = argv[3];

  // Try and load input script text
  std::string script_text_raw;
  if (!mg::fs::read_file(input_mrg, script_text_raw)) {
    fprintf(stderr, "Failed to read script text from '%s'\n", input_mrg);
    return -1;
  }

  // Attempt to parse script text as MZP archive
  mg::data::Mzp mzp;
  if (!mg::data::mzp_read(script_text_raw, mzp)) {
    fprintf(stderr, "Failed to parse script data as MZP\n");
    return -1;
  }

  // Try and load the translation DB
  std::string translation_db_raw;
  if (!mg::fs::read_file(translation_db_filename, translation_db_raw)) {
    fprintf(stderr, "Failed to translation db from '%s'\n",
            translation_db_filename);
    return -1;
  }
  nlohmann::json translation_db = nlohmann::json::parse(translation_db_raw);

  // Retranslate the script
  mg::data::Mzp translated_mzp = patch_string_table(translation_db, mzp);

  // Write out the translated archive
  std::string mzp_out;
  mg::data::mzp_write(translated_mzp, mzp_out);
  if (!mg::fs::write_file(output_mrg, mzp_out)) {
    return -1;
  }

  return 0;
}
