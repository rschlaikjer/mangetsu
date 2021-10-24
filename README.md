# Mangetsu

A collection of tools for reading/writing the data formats packaged in the
Nintendo Switch version of the Tsukihime Remake.

## Building

To build all tools, you will need at least the following

```bash
sudo apt install -y build-essential cmake zlib1g-dev libssl-dev
```

If you also with to build the graphical tools, you will need some additional
dependencies

```bash
sudo apt install -y libopengl-dev libglfw3-dev
```

To actually build the tools:

```bash
git clone git@github.com:rschlaikjer/mangetsu.git
cd mangetsu
mkdir build && cd build
cmake ..                # No UI programs
cmake -DBUILD_GUI ..    # With UI programs
make
```

## Tool Overview

### MRG files

The MRG files operate on the mrg/hed/nam file triplets.
MRG files are archive formats, and can be unzipped / zipped using
the tools with the `mrg_` prefix. Nam files are optional - when found, they
will be used.

- `mrg_info`: Print the file list contained in a mrg. May optionally output in
  machine readable format
- `mrg_extract`: Unpack all files in a mrg. If a nam file is present, filenames
  will include the nam entry.
- `mrg_pack`: Construct a new mrg/hed/nam from individual files. Files will be
  packed in the order they are specified.
- `mrg_replace`: Given a base mrg/hed, create a new mrg/hed with archive
  entries at certain offsets in the original file replaced by new files.
- `nam_read`: Print the names in a nam file.

### MZP files

Like MRG files, MZPs are archive formats that contain multiple sections. Unlike
MRG, these files are self-describing and do not have a separate HED file.

- `mzp_info`: List information about existing mzp file
- `mzp_compress`: Combine multiple files into an mzp archive
- `mzp_extract`: Extract all sections from an mzp archive

### MZX

MZX is a basic LZ-adjacent compression format. It is purely a compression
format, with no archive capabilities.

- `mzx_decompress`: Decompress a MZX-compressed file
- `mzx_compress`: Compress a raw file using MZX compression. NOTE: This program
currently does not attempt to actually do _useful_ compression - it will
generate a valid MZP output, but the output _will_ be larger than the input
file.

### NXX

NXGX / NXCX files are GZIP / LZ compressed data formats with a small header.
Note that NXCX support is not tested due to lack of sample files.

- `nxx_decompress`: Given a file in either NXGZ or NXCX format, uncompress the
  data to a new file.
- `nxgx_compress`: Given a raw file, compress in NXGZ format.

### GUI Programs

If GUI support is enabled, the `data_explorer` file will be built. This UI
allows reinterpreting a file as any of the formats above on the fly, as well as
recursively extracting and displaying sub-archives of those files. It also
features a hex view and other tools designed to make analysis of raw formats
easier.
