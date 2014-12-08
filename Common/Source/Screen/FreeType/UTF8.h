/* 
 * File:   UTF8.h
 * Author: bruno
 *
 * Created on 8 décembre 2014, 01:16
 */

#ifndef UTF8_H
#define	UTF8_H

#include "utils/utf8/unchecked.h"
#include "string.h"

static std::pair<unsigned, const char *>
NextUTF8(const char *p) {
    return std::make_pair(utf8::unchecked::next(p), p);
}

bool ValidateUTF8(const char *p) {
  size_t len = strlen(p);
  return (utf8::find_invalid(p, p + len) == (p + len));
}

#endif	/* UTF8_H */

