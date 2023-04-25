// Copyright (c) 2023, Bruno de Lacheisserie
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of  nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "fdsan.h"
#include <unistd.h>

enum android_fdsan_error_level
android_fdsan_set_error_level(enum android_fdsan_error_level new_level) __attribute__((__weak__));

void android_fdsan_exchange_owner_tag(int fd, uint64_t expected_tag,
                                      uint64_t new_tag) __attribute__((__weak__));

int android_fdsan_close_with_tag(int fd, uint64_t tag) __attribute__((__weak__));

uint64_t fdsan_owner_tag(const void* obj) {
    return (uint64_t)(obj);
}

void fdsan_set_error_level(enum android_fdsan_error_level new_level) {
    if (android_fdsan_set_error_level) {
        android_fdsan_set_error_level(new_level);
    }
}

void fdsan_exchange_owner(int fd, uint64_t expected_tag, uint64_t new_tag) {
    if (android_fdsan_exchange_owner_tag) {
        android_fdsan_exchange_owner_tag(fd, expected_tag, new_tag);
    }
}

int fdsan_close(int fd, uint64_t tag) {
    if (android_fdsan_close_with_tag) {
        return android_fdsan_close_with_tag(fd, tag);
    } else {
        return close(fd);
    }
}
