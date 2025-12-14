#pragma once
#include <streambuf>
#include <utility>
#include <array>
#include "openzip.h"
#include "tchar.h"

class zzip_file_stream : public std::streambuf {

 public:
  zzip_file_stream(zzip_file_ptr&& fp) : m_fp(std::move(fp)) {}

  zzip_file_stream(const TCHAR* szFile, const char* mode);

  zzip_file_stream() = default;
  zzip_file_stream(const zzip_file_stream&) = delete;
  zzip_file_stream& operator=(const zzip_file_stream&) = delete;

  zzip_file_stream(zzip_file_stream&& other) noexcept
      : std::streambuf(other),
        m_fp(std::exchange(other.m_fp, nullptr)),
        m_buffer(other.m_buffer) {}

  zzip_file_stream& operator=(zzip_file_stream&& other) noexcept;

  operator bool() const {
    return static_cast<bool>(m_fp);
  }

 protected:
  int_type underflow() override;

 private:
  zzip_file_ptr m_fp;
  std::array<char, 8192> m_buffer;
};
