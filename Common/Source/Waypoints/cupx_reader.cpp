#include "cupx_reader.h"
#include "utils/zzip_mem_disk.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>

namespace {

// Helpers to read little-endian integers from a byte buffer.
// Using explicit byte reads keeps the code correct on both little- and
// big-endian targets and avoids alignment/UB issues that can occur when casting
// pointers.
uint32_t read_le32(const std::span<const char>& data, size_t pos) {
  if (pos + 4 > data.size()) {
    throw std::out_of_range("read_le32: insufficient data");
  }
  return static_cast<uint32_t>(static_cast<uint8_t>(data[pos])) |
         (static_cast<uint32_t>(static_cast<uint8_t>(data[pos + 1])) << 8) |
         (static_cast<uint32_t>(static_cast<uint8_t>(data[pos + 2])) << 16) |
         (static_cast<uint32_t>(static_cast<uint8_t>(data[pos + 3])) << 24);
}

uint16_t read_le16(const std::span<const char>& data, size_t pos) {
  if (pos + 2 > data.size()) {
    throw std::out_of_range("read_le16: insufficient data");
  }
  return static_cast<uint16_t>(static_cast<uint8_t>(data[pos])) |
         (static_cast<uint16_t>(static_cast<uint8_t>(data[pos + 1])) << 8);
}

// Find all ZIP signatures in a buffer
std::vector<size_t> find_zip_signatures(std::span<const char> data) {
  // EOCD = End Of Central Directory
  constexpr uint8_t EOCD_SIG[4] = {0x50, 0x4B, 0x05, 0x06};

  auto it = std::search(data.begin(), data.end(), std::begin(EOCD_SIG),
                        std::end(EOCD_SIG));

  // If no EOCD found, return a single archive starting at 0
  if (it == data.end()) {
    return std::vector<size_t>({0U});
  }

  size_t eocd_pos = static_cast<size_t>(std::distance(data.begin(), it));

  // Need at least the minimum EOCD size (22 bytes) available
  if (eocd_pos + 22 > data.size()) {
    return std::vector<size_t>({0U});
  }

  // EOCD structure (relative to EOCD start):
  // Offset 12: 4 bytes = size of central directory
  // Offset 16: 4 bytes = offset of start of central directory
  // Offset 20: 2 bytes = comment length

  // ZIP format uses little-endian for multi-byte fields in the EOCD.
  // Read explicitly as little-endian to be portable across architectures.
  uint32_t cdSize = read_le32(data, eocd_pos + 12);
  uint32_t cdOffset = read_le32(data, eocd_pos + 16);
  uint16_t commentLen = read_le16(data, eocd_pos + 20);

  // EOCD size includes the 22-byte base + comment length
  size_t eocd_size = 22 + static_cast<size_t>(commentLen);

  // End of pics.zip = start of points.zip =
  //          start of central directory + size of central directory + eocd size

  return {0, cdOffset + cdSize + eocd_size};
}

}  // namespace

cupx_reader::cupx_reader(const tstring& path) {
  // Read entire file into memory via memory-mapped file
  m_mmf.open(path.c_str(), true);

  if (!m_mmf.is_open()) {
    throw std::runtime_error(std::string("Cannot open CUPX file: " + to_utf8(path.c_str())).c_str());
  }

  auto offsets = find_zip_signatures({m_mmf.data(), m_mmf.file_size()});

  for (size_t i = 0; i < offsets.size(); ++i) {
    size_t start = offsets[i];
    size_t end = (i + 1 < offsets.size()) ? offsets[i + 1] : m_mmf.file_size();

    zip_archive archive(m_mmf.data() + start, end - start);

    m_archives.push_back(std::move(archive));
  }
}

// Read POINTS.CUP
zzip_disk_file_stream cupx_reader::read_points_cup() const {
  for (const auto& archive : m_archives) {
    try {
      zzip_mem_disk disk(archive);
      return disk.get_file("POINTS.CUP");
    }
    catch (const std::exception&) {
      continue;
    }
  }
  throw std::runtime_error("POINTS.CUP not found in CupX file");
}

zzip_disk_file_stream cupx_reader::read_image(const std::string& filename) const {
  for (const auto& archive : m_archives) {
    try {
      zzip_mem_disk disk(archive);
      return disk.get_file("pics/" + filename);
    }
    catch (const std::exception&) {
      continue;
    }
  }
  throw std::runtime_error(filename + " not found in CupX file");
}
