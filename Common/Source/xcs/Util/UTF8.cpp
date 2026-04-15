/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "options.h"
#include "Util/UTF8.hpp"
#include "Util/CharUtil.hpp"

#include <algorithm>

#include <assert.h>

/**
 * Is this a leading byte that is followed by 1 continuation byte?
 */
static constexpr bool
IsLeading1(unsigned char ch)
{
  return (ch & 0xe0) == 0xc0;
}

static constexpr unsigned char
MakeLeading1(unsigned char value)
{
  return 0xc0 | value;
}

/**
 * Is this a leading byte that is followed by 2 continuation byte?
 */
static constexpr bool
IsLeading2(unsigned char ch)
{
  return (ch & 0xf0) == 0xe0;
}

static constexpr unsigned char
MakeLeading2(unsigned char value)
{
  return 0xe0 | value;
}

/**
 * Is this a leading byte that is followed by 3 continuation byte?
 */
static constexpr bool
IsLeading3(unsigned char ch)
{
  return (ch & 0xf8) == 0xf0;
}

static constexpr unsigned char
MakeLeading3(unsigned char value)
{
  return 0xf0 | value;
}

/**
 * Is this a leading byte that is followed by 4 continuation byte?
 */
static constexpr bool
IsLeading4(unsigned char ch)
{
  return (ch & 0xfc) == 0xf8;
}

static constexpr bool
IsContinuation(unsigned char ch)
{
  return (ch & 0xc0) == 0x80;
}

/**
 * Generate a continuation byte of the low 6 bit.
 */
static constexpr unsigned char
MakeContinuation(unsigned char value)
{
  return 0x80 | (value & 0x3f);
}

static constexpr bool
IsValidUnicodeScalar(unsigned ch)
{
  return ch <= 0x10ffff && (ch < 0xd800 || ch > 0xdfff);
}

static const char *
FindNonASCIIOrZero(const char *p);

bool
ValidateUTF8(const char *p)
{
  while (true) {
    /* fast path for ASCII runs; compilers can auto-vectorize this */
    p = FindNonASCIIOrZero(p);

    const unsigned char ch = *p;
    if (ch == 0)
      return true;

    ++p;

    if (gcc_unlikely(ch < 0xc2))
      /* continuation without a prefix, overlong two-byte sequence or
         some other illegal start byte */
      return false;

    if (ch < 0xe0) {
      /* 1 continuation */
      if (!IsContinuation(*p))
        return false;

      ++p;
    } else if (ch < 0xf0) {
      /* 2 continuations */
      const unsigned char a = *p;
      if (!IsContinuation(a))
        return false;

      ++p;

      if ((ch == 0xe0 && a < 0xa0) ||
          (ch == 0xed && a >= 0xa0))
        /* overlong sequence or UTF-16 surrogate */
        return false;

      if (!IsContinuation(*p))
        return false;

      ++p;
    } else if (ch < 0xf5) {
      /* 3 continuations */
      const unsigned char a = *p;
      if (!IsContinuation(a))
        return false;

      ++p;

      if ((ch == 0xf0 && a < 0x90) ||
          (ch == 0xf4 && a >= 0x90))
        /* overlong sequence or code point above U+10FFFF */
        return false;

      if (!IsContinuation(*p) || !IsContinuation(*(p + 1)))
        return false;

      p += 2;
    } else {
      /* RFC 3629 UTF-8 ends at four bytes */
      return false;
    }
  }
}

size_t
SequenceLengthUTF8(char ch)
{
  const unsigned char uch = ch;

  if (IsASCII(uch))
    return 1;
  else if (uch >= 0xc2 && uch < 0xe0)
    /* 1 continuation */
    return 2;
  else if (uch >= 0xe0 && uch < 0xf0)
    /* 2 continuations */
    return 3;
  else if (uch >= 0xf0 && uch < 0xf5)
    /* 3 continuations */
    return 4;
  else
    /* continuation without a prefix or some other illegal
       start byte */
    return 0;
}

template<size_t L>
struct CheckSequenceUTF8 {
  gcc_pure
  bool operator()(const char *p) const {
    return IsContinuation(*p) && CheckSequenceUTF8<L-1>()(p + 1);
  }
};

template<>
struct CheckSequenceUTF8<0u> {
  constexpr bool operator()(gcc_unused const char *p) const {
    return true;
  }
};

template<size_t L>
gcc_pure
static size_t
InnerSequenceLengthUTF8(const char *p)
{
  return CheckSequenceUTF8<L>()(p)
    ? L + 1
    : 0u;
}

size_t
SequenceLengthUTF8(const char *p)
{
  const unsigned char ch = *p++;

  if (IsASCII(ch))
    return 1;
  else if (ch < 0xc2)
    return 0;
  else if (ch < 0xe0)
    return IsContinuation(*p) ? 2 : 0;
  else if (ch < 0xf0) {
    const unsigned char a = *p;
    if (!IsContinuation(a) ||
        (ch == 0xe0 && a < 0xa0) ||
        (ch == 0xed && a >= 0xa0))
      return 0;

    ++p;
    return IsContinuation(*p) ? 3 : 0;
  } else if (ch < 0xf5) {
    const unsigned char a = *p;
    if (!IsContinuation(a) ||
        (ch == 0xf0 && a < 0x90) ||
        (ch == 0xf4 && a >= 0x90))
      return 0;

    ++p;
    if (!IsContinuation(*p))
      return 0;

    ++p;
    return IsContinuation(*p) ? 4 : 0;
  } else
    return 0;
}

static const char *
FindNonASCIIOrZero(const char *p)
{
  while (*p != 0 && IsASCII(*p))
    ++p;
  return p;
}

char *
Latin1ToUTF8(unsigned char ch, char *buffer)
{
  if (IsASCII(ch)) {
    *buffer++ = ch;
  } else {
    *buffer++ = MakeLeading1(ch >> 6);
    *buffer++ = MakeContinuation(ch);
  }

  return buffer;
}

const char *
Latin1ToUTF8(const char *gcc_restrict src, char *gcc_restrict buffer,
             size_t buffer_size)
{
  const char *p = FindNonASCIIOrZero(src);
  if (*p == 0)
    /* everything is plain ASCII, we don't need to convert anything */
    return src;

  if ((size_t)(p - src) >= buffer_size)
    /* buffer too small */
    return nullptr;

  const char *const end = buffer + buffer_size;
  char *q = std::copy(src, p, buffer);

  while (*p != 0) {
    unsigned char ch = *p++;

    if (IsASCII(ch)) {
      *q++ = ch;

      if (q >= end)
        /* buffer too small */
        return nullptr;
    } else {
      if (q + 2 >= end)
        /* buffer too small */
        return nullptr;

      *q++ = MakeLeading1(ch >> 6);
      *q++ = MakeContinuation(ch);
    }
  }

  *q = 0;
  return buffer;
}

char *
UnicodeToUTF8(unsigned ch, char *q)
{
  if (gcc_unlikely(!IsValidUnicodeScalar(ch)))
    ch = 0xfffdu;

  if (gcc_likely(ch < 0x80)) {
    *q++ = (char)ch;
  } else if (gcc_likely(ch < 0x800)) {
    *q++ = MakeLeading1(ch >> 6);
    *q++ = MakeContinuation(ch);
  } else if (ch < 0x10000) {
    *q++ = MakeLeading2(ch >> 12);
    *q++ = MakeContinuation(ch >> 6);
    *q++ = MakeContinuation(ch);
  } else {
    *q++ = MakeLeading3(ch >> 18);
    *q++ = MakeContinuation(ch >> 12);
    *q++ = MakeContinuation(ch >> 6);
    *q++ = MakeContinuation(ch);
  }

  return q;
}

size_t
LengthUTF8(const char *p)
{
  /* this is a very naive implementation: it does not do any
     verification, it just counts the bytes that are not a UTF-8
     continuation  */

  size_t n = 0;
  for (; *p != 0; ++p)
    if (!IsContinuation(*p))
      ++n;
  return n;
}

/**
 * Find the null terminator.
 */
gcc_pure
static char *
FindTerminator(char *p)
{
  assert(p != nullptr);

  while (*p != 0)
    ++p;

  return p;
}

/**
 * Find the leading byte for the given continuation byte.
 */
gcc_pure
static char *
FindLeading(gcc_unused char *const begin, char *i)
{
  assert(i > begin);
  assert(IsContinuation(*i));

  while (IsContinuation(*--i)) {
    assert(i > begin);
  }

  return i;
}

char *
CropIncompleteUTF8(char *const p)
{
  char *const end = FindTerminator(p);
  if (end == p)
    return end;

  char *const last = end - 1;
  if (!IsContinuation(*last)) {
    char *result = end;
    if (!IsASCII(*last)) {
      *last = 0;
      result = last;
    }

    assert(ValidateUTF8(p));
    return result;
  }

  char *const leading = FindLeading(p, last);
  const size_t n_continuations = last - leading;
  assert(n_continuations > 0);

  const unsigned char ch = *leading;

  unsigned expected_continuations;
  if (IsLeading1(ch))
    expected_continuations = 1;
  else if (IsLeading2(ch))
    expected_continuations = 2;
  else if (IsLeading3(ch))
    expected_continuations = 3;
  else if (IsLeading4(ch))
    expected_continuations = 4;
  else {
    assert(n_continuations == 0);
    gcc_unreachable();
  }

  assert(n_continuations <= expected_continuations);

  char *result = end;

  if (n_continuations < expected_continuations) {
    /* this continuation is incomplete: truncate here */
    *leading = 0;
    result = leading;
  }

  /* now the string must be completely valid */
  assert(ValidateUTF8(p));

  return result;
}

size_t
TruncateStringUTF8(const char *p, size_t max_chars, size_t max_bytes)
{
#if !CLANG_CHECK_VERSION(3,6)
  /* disabled on clang due to -Wtautological-pointer-compare */
  assert(p != nullptr);
#endif
  assert(ValidateUTF8(p));

  size_t result = 0;
  while (max_chars > 0 && *p != '\0') {
    const size_t sequence = SequenceLengthUTF8(p);
    if (sequence == 0 || sequence > max_bytes)
      break;

    result += sequence;
    max_bytes -= sequence;
    p += sequence;
    --max_chars;
  }

  return result;
}

char *
CopyTruncateStringUTF8(char *dest, size_t dest_size,
                       const char *src, size_t truncate)
{
  assert(dest != nullptr);
  assert(dest_size > 0);
  assert(src != nullptr);
  assert(ValidateUTF8(src));

  size_t copy = TruncateStringUTF8(src, truncate, dest_size - 1);
  auto *p = std::copy_n(src, copy, dest);
  *p = '\0';
  return p;
}

std::pair<unsigned, const char *>
NextUTF8(const char *p)
{
  const unsigned char a = *p++;
  if (a == 0)
    return std::make_pair(0u, nullptr);

  if (IsASCII(a))
    return std::make_pair(unsigned(a), p);

  if (gcc_unlikely(a < 0xc2))
    return std::make_pair(0xfffdu, p);

  if (a < 0xe0) {
    /* 1 continuation */
    const unsigned char b = *p;
    if (!IsContinuation(b))
      return std::make_pair(0xfffdu, p);

    ++p;
    return std::make_pair(((a & 0x1f) << 6) | (b & 0x3f), p);
  } else if (a < 0xf0) {
    /* 2 continuations */
    const unsigned char b = *p;
    if (!IsContinuation(b) ||
        (a == 0xe0 && b < 0xa0) ||
        (a == 0xed && b >= 0xa0))
      return std::make_pair(0xfffdu, p);

    ++p;
    const unsigned char c = *p;
    if (!IsContinuation(c))
      return std::make_pair(0xfffdu, p);

    ++p;
    return std::make_pair(((a & 0xf) << 12) | ((b & 0x3f) << 6) | (c & 0x3f),
                          p);
  } else if (a < 0xf5) {
    /* 3 continuations */
    const unsigned char b = *p;
    if (!IsContinuation(b) ||
        (a == 0xf0 && b < 0x90) ||
        (a == 0xf4 && b >= 0x90))
      return std::make_pair(0xfffdu, p);

    ++p;
    const unsigned char c = *p;
    if (!IsContinuation(c))
      return std::make_pair(0xfffdu, p);

    ++p;
    const unsigned char d = *p;
    if (!IsContinuation(d))
      return std::make_pair(0xfffdu, p);

    ++p;
    const unsigned ch = ((a & 0x7) << 18) | ((b & 0x3f) << 12)
      | ((c & 0x3f) << 6) | (d & 0x3f);
    return std::make_pair(IsValidUnicodeScalar(ch) ? ch : 0xfffdu, p);
  } else {
    return std::make_pair(0xfffdu, p);
  }
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("ValidateUTF8 rejects malformed sequences") {
  CHECK(ValidateUTF8("ASCII only"));
  CHECK(ValidateUTF8("κόσμε"));

  CHECK_FALSE(ValidateUTF8("\x80"));
  CHECK_FALSE(ValidateUTF8("\xc2"));
  CHECK_FALSE(ValidateUTF8("\xe0\xa0"));
  CHECK_FALSE(ValidateUTF8("\xf0\x90\x80"));
  CHECK_FALSE(ValidateUTF8("\xc0\x80"));
  CHECK_FALSE(ValidateUTF8("\xc1\xbf"));
  CHECK_FALSE(ValidateUTF8("\xe0\x80\x80"));
  CHECK_FALSE(ValidateUTF8("\xed\xa0\x80"));
  CHECK_FALSE(ValidateUTF8("\xf0\x80\x80\x80"));
  CHECK_FALSE(ValidateUTF8("\xf4\x90\x80\x80"));
  CHECK_FALSE(ValidateUTF8("\xf8\x88\x80\x80\x80"));
}

TEST_CASE("UTF8 helpers stay strict and consistent") {
  CHECK(SequenceLengthUTF8('A') == 1);
  CHECK(SequenceLengthUTF8("κ") == 2);
  CHECK(SequenceLengthUTF8("€") == 3);
  CHECK(SequenceLengthUTF8("🚁") == 4);

  CHECK(SequenceLengthUTF8('\xc0') == 0);
  CHECK(SequenceLengthUTF8('\xf8') == 0);
  CHECK(SequenceLengthUTF8("\xed\xa0\x80") == 0);
  CHECK(SequenceLengthUTF8("\xf4\x90\x80\x80") == 0);

  char surrogate_buffer[8] = {};
  char *surrogate_end = UnicodeToUTF8(0xd800u, surrogate_buffer);
  *surrogate_end = 0;
  CHECK(ValidateUTF8(surrogate_buffer));
  CHECK(NextUTF8(surrogate_buffer).first == 0xfffdu);

  char range_buffer[8] = {};
  char *range_end = UnicodeToUTF8(0x110000u, range_buffer);
  *range_end = 0;
  CHECK(ValidateUTF8(range_buffer));
  CHECK(NextUTF8(range_buffer).first == 0xfffdu);
}
#endif
