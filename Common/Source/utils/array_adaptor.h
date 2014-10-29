/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   array_adaptor.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 26 octobre 2014, 21:08
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

    template<std::size_t N>
    const_array_adaptor(T(&array)[N]) : _array(array), _size(N) {
    }

    const_array_adaptor(const T* array, size_type size) : _array(array), _size(size) {
    }

    constexpr size_type size() const {
        return _size;
    }

    constexpr bool empty() const {
        return _size == 0;
    }

    const_reference operator[](size_type i) const {
        return _array[i];
    }

    //iterator
    typedef const T* const_iterator;

    const_iterator begin() const {
        return _array;
    }

    const_iterator end() const {
        return _array + _size;
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
    const pointer _array;
    const size_type _size;
};

#endif	/* ARRAY_ADAPTOR_H */

