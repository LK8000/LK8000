/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   strcpy.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 21 novembre 2017, 22:18
 */

#ifndef STRCPY_H
#define STRCPY_H

#include "Compiler.h"
#include <cassert>
#include "Util/UTF8.hpp"

namespace lk {

template <typename CharT>
void strcpy(CharT* gcc_restrict dst, const CharT* gcc_restrict src, size_t dst_size) {
  assert(dst);
  assert(src);
  assert(dst_size);

  CharT* out = dst;
  while (--dst_size != 0) {
    if ((*out++ = *src++) == 0) {
      break;
    }
  }
  if (dst_size == 0) {
    // out string to small
    *out = '\0'; /* NULL-terminate dst */
    if constexpr (std::is_same_v<CharT, char>) {
      CropIncompleteUTF8(dst);
    }
  }
}

template <typename CharT, size_t dst_size>
void strcpy(CharT (&dst)[dst_size], const CharT* src) {
  lk::strcpy(dst, src, dst_size);
}

}  // namespace lk

#endif /* STRCPY_H */
