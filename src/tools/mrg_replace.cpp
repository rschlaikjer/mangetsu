#include <filesystem>
#include <map>

#include <mg.hpp>
#include <mg/data/mrg.hpp>
#include <mg/data/nam.hpp>
#include <mg/data/nxx.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

void usage(const char *program_name) {
  fprintf(
      stderr,
      "%s -iINDEX1 file1 [-iINDEX2 file2...] input_basename output_basename\n",
      program_name);
}

int main(int argc, char **argv) {
  // Parse args
  std::map<long, const char *> replace_indices;
  const char *input_basename = nullptr;
  const char *output_basename = nullptr;

  for (int i = 1; i < argc; i++) {
    if (!strncmp("-i", argv[i], 2)) {
      // Check that the rest of the string is a valid number
      char *endptr;
      long replace_idx = strtol(argv[i] + 2, &endptr, 0);
      if (endptr == argv[i] + 2) {
        fprintf(stderr, "Failed to parse index '%s'\n", argv[i] + 2);
        return -1;
      }

      // Do we have a filename to go in this slot
      if (i + 1 >= argc) {
        fprintf(stderr, "No file provided for replacement at index %ld\n",
                replace_idx);
        return -1;
      }

      // Add to extract set
      replace_indices[replace_idx] = argv[i + 1];

      // Skip arg and loop
      i++;
      continue;
    }

    // Basenames?
    if (input_basename == nullptr) {
      input_basename = argv[i];
      continue;
    }
    if (output_basename == nullptr) {
      output_basename = argv[i];
      continue;
    }

    // Invalid state
    usage(argv[0]);
    return -1;
  }

  // Check args are OK
  if (input_basename == nullptr || output_basename == nullptr) {
    usage(argv[0]);
    return -1;
  }
  if (replace_indices.size() == 0) {
    fprintf(stderr, "No files to replace - exiting\n");
    return -1;
  }

  // Load each of the replace files
  std::map<long, std::unique_ptr<mg::fs::MappedFile>> replacement_files;
  for (auto &[index, filename] : replace_indices) {
    auto mapped = mg::fs::MappedFile::open(filename);
    if (mapped == nullptr) {
      return -1;
    }
    replacement_files[index] = std::move(mapped);
  }

  // Check for the original mrg/hed files
  const std::string input_hed_filename =
      mg::string::format("%s.hed", input_basename);
  const std::string input_mrg_filename =
      mg::string::format("%s.mrg", input_basename);

  // Read raw data for hed file
  std::string hed_raw;
  if (!mg::fs::read_file(input_hed_filename.c_str(), hed_raw)) {
    return -1;
  }

  // Try and map the mrg data
  std::shared_ptr<mg::fs::MappedFile> mrg_data =
      mg::fs::MappedFile::open(input_mrg_filename.c_str());
  if (mrg_data == nullptr) {
    return -1;
  }

  // Parse the MRG data
  auto mrg = mg::data::MappedMrg::parse(hed_raw, mrg_data);
  if (mrg == nullptr) {
    return -1;
  }

  // Make output names
  const std::string output_hed_filename =
      mg::string::format("%s.hed", output_basename);
  const std::string output_mrg_filename =
      mg::string::format("%s.mrg", output_basename);

  // Open hed output
  const int hed_fd =
      open(output_hed_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (hed_fd == -1) {
    fprintf(stderr, "Failed to open '%s' - %s\n", output_hed_filename.c_str(),
            strerror(errno));
    return -1;
  }
  std::shared_ptr<void> _defer_close_hed_fd(nullptr,
                                            [=](...) { close(hed_fd); });

  // Open mrg output
  const int mrg_fd =
      open(output_mrg_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (mrg_fd == -1) {
    fprintf(stderr, "Failed to open '%s' - %s\n", output_mrg_filename.c_str(),
            strerror(errno));
    return -1;
  }
  std::shared_ptr<void> _defer_close_mrg_fd(nullptr,
                                            [=](...) { close(mrg_fd); });

  // Padding buffer for rounding to the nearest segment
  char padding[mg::data::Mrg::SECTOR_SIZE];
  memset(padding, 0x0, sizeof(padding));

  // Generate our new hed/mrg files
  ssize_t mrg_write_offset = 0;
  for (unsigned i = 0; i < mrg->entries().size(); i++) {

    // Create the header for this next entry. Offset must always be the new
    // current mrg offset.
    mg::data::Mrg::PackedEntryHeader header;
    header.offset = mrg_write_offset / mg::data::Mrg::SECTOR_SIZE;

    // Do we want to replace this index?
    const bool replace_index = replace_indices.find(i) != replace_indices.end();

    // Calculate the file size in header
    if (replace_index) {
      // Use size / data from the replacement table
      const auto &replacement_data = replacement_files.at(i);
      header.size_sectors =
          mg::data::Mrg::size_in_sectors(replacement_data->size());
      header.size_uncompressed_sectors = header.size_sectors;

      // If this is a known compress format, update the decompressed sector size
      {
        mg::data::Nxx nxx_header;
        if (extract_nxx_header(replacement_data->string_view(), nxx_header)) {
          header.size_uncompressed_sectors =
              mg::data::Mrg::size_in_sectors(nxx_header.size);
        }
      }
    } else {
      // Use the sizes / data from the old header
      const auto &old_header = mrg->entries()[i];
      header.size_sectors = old_header.size_sectors;
      header.size_uncompressed_sectors = old_header.size_uncompressed_sectors;
    }

    // Write the data segment
    if (replace_index) {
      // Write the new data
      const auto &replacement_data = replacement_files.at(i);
      ASSERT(write(mrg_fd, replacement_data->data(),
                   replacement_data->size()) == replacement_data->size());
      mrg_write_offset += replacement_data->size();

      // Pad to the nearest sector
      const ssize_t bytes_to_pad =
          (header.size_sectors * mg::data::Mrg::SECTOR_SIZE) -
          replacement_data->size();
      ASSERT(write(mrg_fd, padding, bytes_to_pad) == bytes_to_pad);
      mrg_write_offset += bytes_to_pad;
    } else {
      const auto old_data = mrg->entry_data(i);
      ASSERT(write(mrg_fd, old_data.data(), old_data.size()) ==
             (ssize_t)old_data.size());
    }

    // Write the hew HED entry
    header.to_file_order();
    ASSERT(write(hed_fd, &header, sizeof(header)) == sizeof(header));
  }

  // Write two all-F HED entries to indicate EOF
  char eof[sizeof(mg::data::Mrg::PackedEntryHeader) * 2];
  memset(eof, 0xFF, sizeof(eof));
  ASSERT(write(hed_fd, eof, sizeof(eof)) == sizeof(eof));

  return 0;
}
