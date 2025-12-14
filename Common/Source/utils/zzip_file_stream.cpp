#include "zzip_file_stream.h"
#include <cassert>

zzip_file_stream::zzip_file_stream(const TCHAR* szFile, const char* mode) {
  m_fp.reset(openzip(szFile, mode));
}

zzip_file_stream& zzip_file_stream::operator=(zzip_file_stream&& other) noexcept {
  std::streambuf::operator=(other);
  std::swap(m_fp, other.m_fp);
  std::swap(m_buffer, other.m_buffer);
  return *this;
}

int zzip_file_stream::underflow() {
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }
  assert(m_fp);

  zzip_ssize_t bytes_read =
      zzip_read(m_fp.get(), m_buffer.data(), m_buffer.size());

  if (bytes_read <= 0) {
    return traits_type::eof();
  }

  setg(m_buffer.data(), m_buffer.data(), m_buffer.data() + bytes_read);

  return traits_type::to_int_type(*gptr());
}
