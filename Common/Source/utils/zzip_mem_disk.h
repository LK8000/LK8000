#pragma once
#include "zzip_disk_file_stream.h"
#include <span>
#include <vector>

// RAII wrapper for ZZIP_DISK (zzipmmapped)
class zzip_mem_disk {
 public:
  explicit zzip_mem_disk(std::span<const char> data);

  ~zzip_mem_disk();

  zzip_mem_disk() = delete;
  zzip_mem_disk(const zzip_mem_disk&) = delete;
  zzip_mem_disk& operator=(const zzip_mem_disk&) = delete;

  zzip_mem_disk(zzip_mem_disk&& other) noexcept
      : m_disk(std::exchange(other.m_disk, nullptr)), m_data(other.m_data) {}

  zzip_mem_disk& operator=(zzip_mem_disk&& other) noexcept;

  ZZIP_DISK* get() const {
    return m_disk;
  }

  zzip_disk_file_stream get_file(const std::string& filename) const;

  class entry_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;

    entry_iterator() : m_disk(nullptr), m_entry(nullptr) {}
    explicit entry_iterator(ZZIP_DISK* disk) : m_disk(disk), m_entry(zzip_disk_findfirst(disk)) {}
    entry_iterator(ZZIP_DISK* disk, ZZIP_DISK_ENTRY* entry) : m_disk(disk), m_entry(entry) {}

    entry_iterator& operator++();
    entry_iterator operator++(int);

    bool operator==(const entry_iterator& other) const {
      return m_entry == other.m_entry;
    }

    bool operator!=(const entry_iterator& other) const {
      return !(*this == other);
    }

    std::string name() const;

    zzip_disk_file_stream file_stream() const;

   private:
    ZZIP_DISK* m_disk = nullptr;
    ZZIP_DISK_ENTRY* m_entry = nullptr;
  };

  entry_iterator begin() const {
    return entry_iterator(m_disk);
  }

  entry_iterator end() const {
    return entry_iterator();
  }

 private:
  ZZIP_DISK* m_disk = nullptr;
  std::span<const char> m_data;  // Non-owning view
};
