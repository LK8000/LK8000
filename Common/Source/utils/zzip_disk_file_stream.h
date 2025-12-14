#pragma once
#include <zzip/mmapped.h>
#include <streambuf>
#include <utility>
#include <array>
#include <stdexcept>

// RAII wrapper for ZZIP_DISK_FILE
class zzip_disk_file_stream : public std::streambuf {
 public:
  zzip_disk_file_stream(ZZIP_DISK* disk, ZZIP_DISK_ENTRY* entry);

  ~zzip_disk_file_stream();

  zzip_disk_file_stream() = delete;

  zzip_disk_file_stream(const zzip_disk_file_stream&) = delete;
  zzip_disk_file_stream& operator=(const zzip_disk_file_stream&) = delete;

  zzip_disk_file_stream(zzip_disk_file_stream&& other) noexcept
      : std::streambuf(),
        m_file(std::exchange(other.m_file, nullptr)),
        m_buffer(other.m_buffer) {
    // Buffer pointers will be set on first underflow() call
  }

  zzip_disk_file_stream& operator=(zzip_disk_file_stream&& other) noexcept;

  ZZIP_DISK_FILE* get() const {
    return m_file;
  }

 protected:
  int_type underflow() override;

 private:
  ZZIP_DISK_FILE* m_file = nullptr;
  std::array<char, 8192> m_buffer;
};
