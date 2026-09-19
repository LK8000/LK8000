/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#ifndef MD5_H
#define MD5_H

#include "md5internal.h"
#include <type_traits>
#include <ranges>
#include <span>
#include "OS/ByteOrder.hpp"

namespace md5_internal {

template <typename T>
concept byte_array = std::ranges::contiguous_range<T> &&
                     sizeof(std::ranges::range_value_t<T>) == 1;

template <typename T>
concept other_container = std::ranges::range<T> && !byte_array<T>;

}  // namespace md5_internal

class MD5_Base {
 protected:
  md5_ctx context;

 public:
  MD5_Base();
  MD5_Base(unsigned key1, unsigned key2, unsigned key3, unsigned key4);

  void Init();
  void Init(unsigned key1, unsigned key2, unsigned key3, unsigned key4);

  // MD5 block update operation. Continues an MD5 message-digest
  // operation, processing another message block, and updating the
  // context.
  void Update(const void* input, size_t size);

  // Arithmetic types (char, int, float, etc.)
  template <typename T>
    requires std::is_arithmetic_v<T>
  void Update(const T& data) {
    static_assert(IsLittleEndian(), "Big-Endian Arch is not supported");
    Update(&data, sizeof(data));
  }

  // Contiguous ranges (std::string, std::vector, std::array, std::span, etc.)
  template <md5_internal::byte_array T>
  void Update(const T& data) {
    auto bytes = std::as_bytes(std::span(data));
    Update(bytes.data(), bytes.size());
  }

  // Non-contiguous ranges (std::list, std::set, etc.)
  template <md5_internal::other_container T>
  void Update(const T& data) {
    for (const auto& item : data) {
      Update(item);
    }
  }
};

class MD5 : public MD5_Base {
 public:
  using MD5_Base::MD5_Base;
  explicit MD5(const MD5_Base& md5) : MD5_Base(md5) {}

  // MD5 finalization. Ends an MD5 message-digest operation, writing the
  // the message digest and zeroizing the context.
  std::string Final();
};

#endif
