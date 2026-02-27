#include "options.h"
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

// Find the start of second zip archive in the buffer
size_t find_zip_end(std::span<const char> data) {
  // EOCD = End Of Central Directory
  constexpr uint8_t EOCD_SIG[4] = {0x50, 0x4B, 0x05, 0x06};

  auto it = std::search(data.begin(), data.end(), std::begin(EOCD_SIG),
                        std::end(EOCD_SIG));

  // If no EOCD found, return the end of file
  if (it == data.end()) {
    return data.size();
  }

  size_t eocd_pos = static_cast<size_t>(std::distance(data.begin(), it));

  // Need at least the minimum EOCD size (22 bytes) available
  if (eocd_pos + 22 > data.size()) {
    return data.size();
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

  return cdOffset + cdSize + eocd_size;
}

}  // namespace

cupx_reader::cupx_reader(const tstring& path) {
  // Read entire file into memory via memory-mapped file
  m_mmf.open(path.c_str(), true);

  if (!m_mmf.is_open()) {
    throw std::runtime_error(std::string("Cannot open CUPX file: " + to_utf8(path.c_str())).c_str());
  }

  const char* begin = m_mmf.data();
  const char* end = begin + m_mmf.file_size();

  while (begin < end) {
    size_t zip_size = find_zip_end({begin, end});
    m_archives.push_back(zip_archive(begin, begin + zip_size));
    begin += zip_size;
  }
}

// Read POINTS.CUP
zzip_disk_file_stream cupx_reader::read_points_cup() const {
  for (const auto& archive : m_archives) {
    try {
      // Archive is a span over the memory-mapped file (m_mmf.data()).
      // Safe to pass as span because m_mmf lives as long as cupx_reader,
      // and the returned stream must be consumed before cupx_reader is destroyed.
      return zzip_disk_file_stream(archive, "POINTS.CUP");
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
      // Archive is a span over the memory-mapped file (m_mmf.data()).
      // Safe to pass as span because m_mmf lives as long as cupx_reader,
      // and the returned stream must be consumed before cupx_reader is destroyed.
      return zzip_disk_file_stream(archive, "pics/" + filename);
    }
    catch (const std::exception&) {
      continue;
    }
  }
  throw std::runtime_error(filename + " not found in CupX file");
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include <filesystem>
#include "Defines.h"
#include "LocalPath.h"


TEST_CASE("cupx_reader - CUPX file operations") {
  // Get the test file path
  TCHAR test_file[MAX_PATH];
  LocalPath(test_file, _T(LKD_WAYPOINTS), _T("DEMO.cupx"));
  REQUIRE(std::filesystem::exists(test_file));
 
  SUBCASE("Valid CUPX file initialization") {
    
    cupx_reader reader(test_file);
    // If we get here without exception, initialization succeeded
    CHECK(true);
  }

  SUBCASE("Invalid file path throws exception") {

    CHECK_THROWS_AS(
      cupx_reader(_T("/nonexistent/path/file.cupx")),
      std::runtime_error
    );
  }

  SUBCASE("Read POINTS.CUP from valid CUPX file") {
    cupx_reader reader(test_file);
    
    auto points_stream = reader.read_points_cup();
    // Verify stream is valid by checking if we can read from it
    std::string content = {
        (std::istreambuf_iterator<char>(&points_stream)),
        (std::istreambuf_iterator<char>())
    };
    CHECK(!content.empty());
    CHECK(content.find("IMGD54A.jpg") != std::string::npos);
  }

  SUBCASE("Read image from valid CUPX file") {
    cupx_reader reader(test_file);
    
    auto image_1 = reader.read_image("IMGD54A.jpg");
    std::vector<char> image_buf = {
        (std::istreambuf_iterator<char>(&image_1)),
        (std::istreambuf_iterator<char>())
    };
    CHECK(!image_buf.empty());
  }

  SUBCASE("Read non-existent image from valid CUPX file") {
    cupx_reader reader(test_file);
    
    CHECK_THROWS_AS(
      reader.read_image("invalid.jpg"),
      std::runtime_error
    );
  }
}

#endif
