#include "zzip_file_stream.h"
#include "zzip/zzip.h"
#include <utility>

zzip_file_stream::zzip_file_stream(const TCHAR* szFile, const char* mode) {
  m_fp.reset(openzip(szFile, mode));
}

zzip_file_stream::zzip_file_stream(zzip_file_stream&& other) noexcept
    : std::streambuf(other),
      m_fp(std::exchange(other.m_fp, nullptr)),
      m_buffer(std::move(other.m_buffer)) {

  if (other.gptr() != nullptr && other.egptr() != nullptr) {
    auto off = static_cast<off_type>(other.egptr() - other.gptr());
    zzip_seek(m_fp.get(), -off, SEEK_CUR);
  }

  // Reset both get areas; underflow() will re-fill on next read
  setg(nullptr, nullptr, nullptr);
  other.setg(nullptr, nullptr, nullptr);
}


zzip_file_stream& zzip_file_stream::operator=(zzip_file_stream&& other) noexcept {
  std::swap(m_fp, other.m_fp);
  std::swap(m_buffer, other.m_buffer);

  if (other.gptr() != nullptr && other.egptr() != nullptr) {
    auto off = static_cast<off_type>(other.egptr() - other.gptr());
    zzip_seek(m_fp.get(), -off, SEEK_CUR);
  }

  // Reset both get areas; underflow() will re-fill on next read
  setg(nullptr, nullptr, nullptr);
  other.setg(nullptr, nullptr, nullptr);
  return *this;
}

zzip_file_stream::int_type zzip_file_stream::underflow() {
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }

  if (!m_fp) {
    return traits_type::eof();
  }

  zzip_ssize_t bytes_read =
      zzip_read(m_fp.get(), m_buffer.data(), m_buffer.size());

  if (bytes_read <= 0) {
    setg(nullptr, nullptr, nullptr);
    return traits_type::eof();
  }

  setg(m_buffer.data(), m_buffer.data(), m_buffer.data() + bytes_read);

  return traits_type::to_int_type(*gptr());
}

zzip_file_stream::pos_type zzip_file_stream::seekoff(
    off_type off, std::ios_base::seekdir way, std::ios_base::openmode which) {
  if (which != std::ios_base::in || !m_fp) {
    return pos_type(off_type(-1));
  }

  int whence = 0;
  if (way == std::ios_base::beg) {
    whence = SEEK_SET;
  } else if (way == std::ios_base::cur) {
    whence = SEEK_CUR;
    // underlying file position is at the end of the buffer, so we need to adjust 
    // the offset to account for the current buffer position by removing the number 
    // of bytes remaining in the buffer from the offset.
    if (gptr() != nullptr && egptr() != nullptr) {
      off -= static_cast<off_type>(egptr() - gptr());
    }
  } else if (way == std::ios_base::end) {
    whence = SEEK_END;
  } else {
    return pos_type(off_type(-1));
  }

  zzip_off_t result = zzip_seek(m_fp.get(), static_cast<zzip_off_t>(off), whence);

  if (result >= 0) {
    // Invalidate buffer after seek; will be refilled by underflow() on next read
    setg(nullptr, nullptr, nullptr);
  }

  return pos_type(off_type(result));
}

zzip_file_stream::pos_type zzip_file_stream::seekpos(
    pos_type sp, std::ios_base::openmode which) {
  return seekoff(sp, std::ios_base::beg, which);
}
