#include "zzip_disk_file_stream.h"

zzip_disk_file_stream::zzip_disk_file_stream(ZZIP_DISK* disk,
                                             ZZIP_DISK_ENTRY* entry)
    : m_file(zzip_disk_entry_fopen(disk, entry)) {
  // Initialize streambuf pointers to null to indicate that the buffer is
  // empty
  setg(nullptr, nullptr, nullptr);

  if (!m_file) {
    throw std::runtime_error("zzip_disk_entry_fopen failed");
  }
}

zzip_disk_file_stream::~zzip_disk_file_stream() {
  if (m_file) {
    zzip_disk_fclose(m_file);
  }
}

zzip_disk_file_stream& zzip_disk_file_stream::operator=(
    zzip_disk_file_stream&& other) noexcept {
  std::streambuf::operator=(std::move(other));
  std::swap(m_file, other.m_file);
  std::swap(m_buffer, other.m_buffer);
  // Reset get-area to force re-read from new buffer
  setg(nullptr, nullptr, nullptr);
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
