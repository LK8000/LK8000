#include "zzip_file_stream.h"
#include <cassert>

zzip_file_stream::zzip_file_stream(const TCHAR* szFile, const char* mode) {
  m_fp.reset(openzip(szFile, mode));
}

zzip_file_stream::zzip_file_stream(zzip_file_stream&& other) noexcept
    : std::streambuf(other),
      m_fp(std::exchange(other.m_fp, nullptr)),
      m_buffer(std::move(other.m_buffer)) {
  // Reset both get areas; underflow() will re-fill on next read
  setg(nullptr, nullptr, nullptr);
  other.setg(nullptr, nullptr, nullptr);  
}


zzip_file_stream& zzip_file_stream::operator=(zzip_file_stream&& other) noexcept {
  std::swap(m_fp, other.m_fp);
  std::swap(m_buffer, other.m_buffer);
  // Reset both get areas; underflow() will re-fill on next read
  setg(nullptr, nullptr, nullptr);
  other.setg(nullptr, nullptr, nullptr);  
  return *this;
}

int zzip_file_stream::underflow() {
  if (!m_fp) {
    return traits_type::eof();
  }

  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }

  zzip_ssize_t bytes_read =
      zzip_read(m_fp.get(), m_buffer.data(), m_buffer.size());

  if (bytes_read <= 0) {
    return traits_type::eof();
  }

  setg(m_buffer.data(), m_buffer.data(), m_buffer.data() + bytes_read);

  return traits_type::to_int_type(*gptr());
}
