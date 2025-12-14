#include "zzip_mem_disk.h"
#include <strings.h>
#include <memory>
#include <array>
#include <istream>

namespace {

int zzip_strcmp(char* a, char* b) {
  return strcasecmp(a, b);
};

}  // namespace

zzip_mem_disk::zzip_mem_disk(std::span<const char> data)
    : m_data(std::move(data)) {
  // Open ZIP from memory using the mmapped API
  m_disk = zzip_disk_buffer(const_cast<char*>(m_data.data()), m_data.size());
  if (!m_disk) {
    throw std::runtime_error("Cannot open ZIP from memory (invalid ZIP data)");
  }
}

zzip_mem_disk::~zzip_mem_disk() {
  if (m_disk) {
    zzip_disk_close(m_disk);
  }
}

zzip_mem_disk& zzip_mem_disk::operator=(zzip_mem_disk&& other) noexcept {
  std::swap(m_disk, other.m_disk);
  std::swap(m_data, other.m_data);
  return *this;
}

zzip_mem_disk::entry_iterator& zzip_mem_disk::entry_iterator::operator++() {
  m_entry = zzip_disk_findnext(m_disk, m_entry);
  return *this;
}

zzip_mem_disk::entry_iterator zzip_mem_disk::entry_iterator::operator++(int) {
  return {m_disk, zzip_disk_findnext(m_disk, m_entry)};
}

std::string zzip_mem_disk::entry_iterator::name() const {
  std::unique_ptr<char, decltype(&free)> name(
      zzip_disk_entry_strdup_name(m_disk, m_entry), &free);
  if (name) {
    return name.get();
  }
  throw std::runtime_error("no file name");
}

zzip_disk_file_stream zzip_mem_disk::entry_iterator::file_stream() const {
  return zzip_disk_file_stream(m_disk, m_entry);
}

zzip_disk_file_stream zzip_mem_disk::get_file(
    const std::string& filename) const {
  ZZIP_DISK_ENTRY* entry = zzip_disk_findfile(
      m_disk, const_cast<char*>(filename.c_str()), nullptr, zzip_strcmp);
  if (!entry) {
    throw std::runtime_error("File not found in zip archive: " + filename);
  }
  return zzip_disk_file_stream(m_disk, entry);
}
