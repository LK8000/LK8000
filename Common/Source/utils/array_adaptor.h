/* Copyright (c) 2018-2020, Bruno de Lacheisserie
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

template <typename T>
class const_array_adaptor {
public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T value_type;
    typedef const T &const_reference;
    typedef const T *pointer;

    template<std::size_t size>
    const_array_adaptor(T(&array)[size]) : _begin(array), _end(array+size) { }

    const_array_adaptor(const T* array, size_type size) : _begin(array), _end(array+size) { }

    const_array_adaptor(const T* begin, const T* end) : _begin(begin), _end(end) { }

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
        return *_begin;
    }

    const_reference back() const {
        return *(_end-1);
    }

    //iterator
    typedef const T* const_iterator;

    const_iterator begin() const {
        return _begin;
    }

    const_iterator end() const {
        return _end;
    }

    // reverse iterator
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }


protected:
    const pointer _begin;
    const pointer _end;
};

template <typename T>
const_array_adaptor<T> make_array(const T* array, size_t size) {
    return const_array_adaptor<T>(array, size);
}

template <typename T, std::size_t size>
const_array_adaptor<T> make_array(T(&array)[size]) {
    return const_array_adaptor<T>(array, size);
}

template <typename T>
const_array_adaptor<T> make_array(const T* begin, const T* end) {
    return const_array_adaptor<T>(begin, end);
}

#endif	/* ARRAY_ADAPTOR_H */
