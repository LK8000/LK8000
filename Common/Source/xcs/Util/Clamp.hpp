/*
 * Copyright (C) 2012 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CLAMP_HPP
#define CLAMP_HPP

#include "Compiler.h"

/**
 * Clamps the specified value in a range.  Returns #min or #max if the
 * value is outside.
 */
template<typename T>
static inline constexpr const T &
Clamp(const T &value, const T &min, const T &max)
{
  return gcc_unlikely(value < min)
    ? min
    : (gcc_unlikely(value > max)
       ? max : value);
}

#if (defined(__ARM_NEON) || defined(__ARM_NEON__)) && !GCC_OLDER_THAN(5,0)

/**
 * don't work with gcc 4.8 and 4.9 ( kobo, openvario )
 */

#include <arm_neon.h>

static inline
int16x4_t Clamp(int16x4_t value, int16x4_t min, int16x4_t max) {
  return vmin_s16(vmax_s16(value, min), max);
}

static inline
int16x8_t Clamp(int16x8_t value, int16x8_t min, int16x8_t max) {
    return vminq_s16(vmaxq_s16(value, min), max);
}

static inline
int32x4_t Clamp(int32x4_t value, int32x4_t min, int32x4_t max) {
    return vminq_s32(vmaxq_s32(value, min), max);
}

#endif

#endif
