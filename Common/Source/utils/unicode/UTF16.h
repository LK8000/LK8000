/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   UTF16.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 13 December 2020, 00:12
 */

#ifndef _utils_unicode_UTF16_h_
#define _utils_unicode_UTF16_h_

#include "Compiler.h"
#include "options.h"
#include <utility>
#include <stdint.h>



/**
 * Decode the next UNICODE character.
 *
 * @param p a null-terminated valid UTF-16 string
 * @return a pair containing the next UNICODE character code and a
 * pointer to the first uint16_t of the following character or 0 if
 * already at the end of the string
 */
gcc_pure gcc_nonnull_all
std::pair<unsigned, const uint16_t *>
NextUTF16(const uint16_t *p);

/**
 * Convert the specified Unicode character to UTF-16 and write it to
 * the buffer.  buffer must have a length of at least 2!
 *
 * @return a pointer to the buffer plus the added uint16_t(s)
 */
gcc_nonnull_all
uint16_t *
UnicodeToUTF16(unsigned ch, uint16_t *buffer);


#endif // _utils_unicode_UTF16_h_
