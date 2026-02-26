#include "zzip_disk_file_stream.h"
#ifdef _WIN32
#include <string.h>
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

namespace {

int zzip_strcmp(char* a, char* b) {
  return strcasecmp(a, b);
};

}  // namespace

zzip_disk_file_stream::zzip_disk_file_stream(std::span<const char> disk_data,
                                             const std::string& filename)
    : m_disk(std::move(disk_data)) {
  // Initialize streambuf pointers to null to indicate that the buffer is
  // empty
  setg(nullptr, nullptr, nullptr);


  ZZIP_DISK_ENTRY* entry = zzip_disk_findfile(
      m_disk.get(), const_cast<char*>(filename.c_str()), nullptr, zzip_strcmp);
  if (!entry) {
    throw std::runtime_error("File not found in zip archive: " + filename);
  }

  m_file = zzip_disk_entry_fopen(m_disk.get(), entry);
  if (!m_file) {
    throw std::runtime_error("zzip_disk_entry_fopen failed");
  }
}

zzip_disk_file_stream::~zzip_disk_file_stream() {
  if (m_file) {
    zzip_disk_fclose(m_file);
  }
}

zzip_disk_file_stream::zzip_disk_file_stream(zzip_disk_file_stream&& other) noexcept
      : m_disk(std::move(other.m_disk)),
      m_file(std::exchange(other.m_file, nullptr)) {
  // Transfer the get-area pointers (eback/gptr/egptr) from other to this.
  // Compute offsets relative to the buffer base so they remain valid after m_buffer is moved.
  char* other_eback = other.eback();
  if (other_eback != nullptr) {
    // Get-area is active in other; transfer it to this
    std::ptrdiff_t eback_offset = other_eback - other.m_buffer.data();
    std::ptrdiff_t gptr_offset = other.gptr() - other.m_buffer.data();
    std::ptrdiff_t egptr_offset = other.egptr() - other.m_buffer.data();
    m_buffer = std::move(other.m_buffer);
    // Apply the same offsets to this->m_buffer (now holding other's data)
    setg(m_buffer.data() + eback_offset, m_buffer.data() + gptr_offset, m_buffer.data() + egptr_offset);
  } else {
    // No get-area was active in other; initialize to empty
    setg(nullptr, nullptr, nullptr);
  }
  // Clear other's get-area to reflect that ownership has transferred
  other.setg(nullptr, nullptr, nullptr);
}

zzip_disk_file_stream& zzip_disk_file_stream::operator=(
    zzip_disk_file_stream&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (m_file) {
    zzip_disk_fclose(m_file);
  }

  m_disk = std::move(other.m_disk);
  m_file = std::exchange(other.m_file, nullptr);
  
  // After swapping m_buffer, transfer the get-area pointers from other to this.
  // Compute offsets relative to the buffer base so they remain valid.
  char* other_eback = other.eback();
  if (other_eback != nullptr) {
    // other has buffered data; transfer the get-area state to this
    std::ptrdiff_t eback_offset = other_eback - other.m_buffer.data();
    std::ptrdiff_t gptr_offset = other.gptr() - other.m_buffer.data();
    std::ptrdiff_t egptr_offset = other.egptr() - other.m_buffer.data();

    m_buffer = std::move(other.m_buffer);
    // this now holds other's m_buffer, so apply offsets to this->m_buffer
    setg(m_buffer.data() + eback_offset, m_buffer.data() + gptr_offset, m_buffer.data() + egptr_offset);
  } else {
    m_buffer = std::move(other.m_buffer);
    // other has no buffered data; this starts with empty get-area
    setg(nullptr, nullptr, nullptr);
  }
  // Clear other's get-area to reflect that its buffer ownership has transferred
  other.setg(nullptr, nullptr, nullptr);
  return *this;
}

zzip_disk_file_stream::int_type zzip_disk_file_stream::underflow() {
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }
  if (!m_file) {
    return traits_type::eof();
  }
  zzip_ssize_t bytes_read =
      zzip_disk_fread(m_buffer.data(), 1, m_buffer.size(), m_file);

  if (bytes_read <= 0) {
    return traits_type::eof();
  }

  setg(m_buffer.data(), m_buffer.data(), m_buffer.data() + bytes_read);

  return traits_type::to_int_type(*gptr());
}
