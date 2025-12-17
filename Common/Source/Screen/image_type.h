#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <concepts>

namespace image_type {

enum class type : uint8_t {
    unknown,
    png,
    jpeg
};

/* ---------------- PNG ---------------- */
inline bool is_valid_png(const uint8_t* d, size_t n) {
  if (n < 33) {
    return false;
  }

  // Signature
  static constexpr uint8_t sig[8] = {0x89, 0x50, 0x4E, 0x47,
                                     0x0D, 0x0A, 0x1A, 0x0A};
  for (int i = 0; i < 8; ++i) {
    if (d[i] != sig[i]) {
      return false;
    }
  }

  // IHDR chunk length (must be 13)
  uint32_t len = (uint32_t(d[8]) << 24) | (uint32_t(d[9]) << 16) |
                 (uint32_t(d[10]) << 8) | (uint32_t(d[11]));

  if (len != 13) {
    return false;
  }

  // Chunk type "IHDR"
  if (!(d[12] == 'I' && d[13] == 'H' && d[14] == 'D' && d[15] == 'R')) {
    return false;
  }

  // width & height must be > 0
  uint32_t w = (uint32_t(d[16]) << 24) | (uint32_t(d[17]) << 16) |
               (uint32_t(d[18]) << 8) | (uint32_t(d[19]));
  uint32_t h = (uint32_t(d[20]) << 24) | (uint32_t(d[21]) << 16) |
               (uint32_t(d[22]) << 8) | (uint32_t(d[23]));

  return w > 0 && h > 0;
}

/* ---------------- JPEG ---------------- */
inline bool is_valid_jpeg(const uint8_t* d, size_t n) {
  if (n < 4) {
    return false;
  }

  // SOI (Start of Image)
  if (d[0] != 0xFF || d[1] != 0xD8) {
    return false;
  }

  size_t i = 2;
  bool found_eoi = false;

  while (i < n) {
    // Search for next FF marker
    if (d[i] != 0xFF) {
      // In JPEG, there can be data between markers
      // Need to search for the next 0xFF
      i++;
      continue;
    }

    // Skip FF padding bytes
    while (i < n && d[i] == 0xFF) {
      i++;
    }

    if (i >= n) {
      break;
    }

    uint8_t marker = d[i];
    i++;

    // EOI (End of Image)
    if (marker == 0xD9) {
      found_eoi = true;
      break;
    }

    // Standalone markers (no length field)
    // TEM (0x01), RST0-RST7 (0xD0-0xD7), SOI (0xD8)
    if (marker == 0x00 || marker == 0x01 ||
        (marker >= 0xD0 && marker <= 0xD8)) {
      continue;
    }

    // Read segment length
    if (i + 1 >= n) {
      break;
    }

    uint16_t seg_len = (static_cast<uint16_t>(d[i]) << 8) | d[i + 1];
    if (seg_len < 2) {
      return false;
    }

    // Skip segment (length includes the 2 length bytes)
    i += seg_len;
  }

  return found_eoi;
}

/* ---------------- Dispatcher ---------------- */

inline type detect(const uint8_t* data, size_t size) {
  if (size >= 8 && data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E &&
      data[3] == 0x47 && is_valid_png(data, size)) {
    return type::png;
  }

  if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF &&
      is_valid_jpeg(data, size)) {
    return type::jpeg;
  }

  return type::unknown;
}

template <typename C>
concept ByteContainer = requires(const C& c) {
  {c.data()}->std::convertible_to<const uint8_t*>;
  {c.size()}->std::convertible_to<size_t>;
};

inline type detect(ByteContainer auto const& c) {
  return detect(c.data(), c.size());
}

}  // namespace image_type
