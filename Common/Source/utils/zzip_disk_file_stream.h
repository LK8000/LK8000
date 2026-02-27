#pragma once
#include <streambuf>
#include <utility>
#include <array>
#include <span>
#include <stdexcept>
#include <string>

#include "zzip_mem_disk.h"

/// RAII wrapper for ZZIP_DISK_FILE.
///
/// This class wraps a zziplib file handle and provides std::streambuf interface
/// for reading compressed files from an in-memory ZIP archive.
///
/// IMPORTANT LIFETIME REQUIREMENT: This class holds a non-owning std::span view
/// of the ZIP archive data (stored in m_disk.m_data). The caller is responsible
/// for ensuring that disk_data remains valid for the entire lifetime of this stream
/// object. This stream must NOT be stored or used after the buffer it references
/// goes out of scope or is deallocated.
///
/// When underflow() is called, it dereferences pointers from the zzip library that
/// reference the underlying buffer. The stream uses zzip_mem_disk's get() to access
/// the ZZIP_DISK* handle, which internally dereferences the non-owning m_data span.
/// If the buffer is freed while the stream exists, underflow() will access freed memory.
///
/// USAGE PATTERN: Create, immediately wrap in std::istream, consume, and destroy
/// within the same scope as the owning buffer.
class zzip_disk_file_stream : public std::streambuf {
 public:
  /// Constructs a stream for reading a file from a ZIP archive.
  ///
  /// @param disk_data  A non-owning view of the ZIP archive data. The caller must
  ///                   ensure this data remains valid for the entire lifetime of
  ///                   this stream object. The span is stored in zzip_mem_disk.m_data.
  /// @param filename   The name of the file within the ZIP archive to open.
  ///
  /// @throws std::runtime_error if the ZIP data is invalid, the file is not found,
  ///         or the file cannot be opened for reading.
  ///
  /// IMPORTANT: Do not store this stream or use it after disk_data is freed.
  /// This is a "stack-local" stream: create and consume it immediately.
  zzip_disk_file_stream(std::span<const char> disk_data, const std::string& filename);

  ~zzip_disk_file_stream();

  zzip_disk_file_stream() = delete;

  zzip_disk_file_stream(const zzip_disk_file_stream&) = delete;
  zzip_disk_file_stream& operator=(const zzip_disk_file_stream&) = delete;

  zzip_disk_file_stream(zzip_disk_file_stream&& other) noexcept;

  zzip_disk_file_stream& operator=(zzip_disk_file_stream&& other) noexcept;

 protected:
  int_type underflow() override;

 private:
  zzip_mem_disk m_disk;
  ZZIP_DISK_FILE* m_file = nullptr;
  std::array<char, 8192> m_buffer;
};
