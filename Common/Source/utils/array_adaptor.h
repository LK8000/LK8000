/* Copyright (c) 2018-2022, Bruno de Lacheisserie
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARRAY_ADAPTOR_H
#define	ARRAY_ADAPTOR_H

#include <cstddef>
#include <iterator>

/**
 * A simple adaptor for arrays.
 */
template <typename T>
class array_adaptor {
public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    array_adaptor(const iterator begin, const iterator end) : _begin(begin), _end(end) { }

    iterator begin() {
        return _begin;
    }

    iterator end() {
        return _end;
    }

    const_iterator begin() const {
        return _begin;
    }

    const_iterator end() const {
        return _end;
    }

    constexpr size_type size() const {
        return std::distance(begin(), end());
    }

    constexpr bool empty() const {
        return begin() == end();
    }

    const_reference operator[](size_type i) const {
        return _begin[i];
    }

    const_reference front() const {
        return *begin();
    }

    const_reference back() const {
        return *(end() - 1);
    }

    reference operator[](size_type i) {
        return _begin[i];
    }

    reference front() {
        return *begin();
    }

    reference back() {
        return *(end() - 1);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

protected:
    const iterator _begin;
    const iterator _end;
};

template <typename T>
array_adaptor<T> make_array(T* begin, T* end) {
    return array_adaptor<T>(begin, end);
}

template <typename T>
array_adaptor<T> make_array(T* array, size_t size) {
    return make_array<T>(array, std::next(array, size));
}

template <typename T, std::size_t size>
array_adaptor<T> make_array(T(&array)[size]) {
    return make_array<T>(array, size);
}

#endif	/* ARRAY_ADAPTOR_H */
