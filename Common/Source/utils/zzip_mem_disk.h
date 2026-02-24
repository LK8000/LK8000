#pragma once
#include <zzip/mmapped.h>
#include <span>
#include <utility>
#include <string>

/// RAII wrapper for ZZIP_DISK (zzipmmapped).
///
/// This class manages a zziplib disk structure that opens a ZIP archive from
/// in-memory data. It holds a non-owning view of the buffer.
///
/// IMPORTANT LIFETIME REQUIREMENT: This class stores a non-owning std::span (m_data)
/// of the ZIP archive buffer. The caller is responsible for ensuring that the buffer
/// outlives the zzip_mem_disk object. The ZZIP_DISK* returned by get() internally
/// holds pointers into this buffer. If the buffer is freed while this object exists,
/// any subsequent zzip operations will access freed memory.
class zzip_mem_disk {
 public:
  /// Constructs a zzip_mem_disk from ZIP archive data.
  ///
  /// @param data  A non-owning view of the ZIP archive data. The caller must ensure
  ///              this data remains valid for the entire lifetime of this object.
  ///              The span is stored in m_data.
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
  std::span<const char> m_data;  // Non-owning view; caller must ensure it outlives m_disk
};
