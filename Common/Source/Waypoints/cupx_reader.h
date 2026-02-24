#pragma once
#include <span>
#include <string>
#include <vector>
#include <optional>
#include "Util/tstring.hpp"
#include "utils/zzip_disk_file_stream.h"

#include "Library/cpp-mmf/memory_mapped_file.hpp"

// class to read CUPX files contents
class cupx_reader {
 public:
  explicit cupx_reader(const tstring& path);

  // Read POINTS.CUP
  zzip_disk_file_stream read_points_cup() const;

  zzip_disk_file_stream read_image(const std::string& filename) const;

 private:
  memory_mapped_file::read_only_mmf m_mmf;

  using zip_archive = std::span<const char>;
  std::vector<zip_archive> m_archives;
};
