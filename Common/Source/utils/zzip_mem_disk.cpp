#include "zzip_mem_disk.h"
#include <memory>
#include <array>
#include <istream>

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
  entry_iterator old = *this;
  ++(*this);
  return old;
}

std::string zzip_mem_disk::entry_iterator::name() const {
  std::unique_ptr<char, decltype(&free)> name(
      zzip_disk_entry_strdup_name(m_disk, m_entry), &free);
  if (name) {
    return name.get();
  }
  throw std::runtime_error("no file name");
}
