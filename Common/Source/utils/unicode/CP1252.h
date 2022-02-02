/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   CP1252.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 02 February 2022
 */
#ifndef _utils_unicode_CP1252_h_
#define _utils_unicode_CP1252_h_

#include "Compiler.h"
#include <utility>


/**
 * Decode the next UNICODE character.
 *
 * @param p a null-terminated valid ANSI string
 * @return a pair containing the next UNICODE character code and a
 * pointer to the first byte of the following character or 0 if
 * already at the end of the string
 */
gcc_pure gcc_nonnull_all
std::pair<unsigned, const char *>
NextACP(const char *p);

#endif
