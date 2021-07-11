/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: array_back_insert_iterator.h
 *
 */

#ifndef _UTILS_BACK_INSERT_ITERATOR_H_
#define _UTILS_BACK_INSERT_ITERATOR_H_
#include <iterator>

template <class _Tp>
class array_back_insert_iterator
    : public std::iterator<std::output_iterator_tag, void, void, void, void> {
protected:
  _Tp *array;
  size_t size;
  size_t elements;
  size_t overflow;

public:
  typedef _Tp value_type;

  explicit array_back_insert_iterator(_Tp *_array, size_t _size)
      : array(_array), size(_size), elements(0), overflow() {}

  size_t length() const { return elements; }

  bool overflowed() const { return overflow; }
  size_t skipped() const { return overflow; }

  array_back_insert_iterator<_Tp> &operator=(_Tp value) {
    if (elements >= size) {
      ++overflow;
    } else {
      array[elements++] = value;
    }
    return *this;
  }

  /** @value: null terminated array to insert. */
  array_back_insert_iterator<_Tp> &operator=(const _Tp *value) {
    while (*value) {
      (*this) = *(value++);
    }
    return *this;
  }

  array_back_insert_iterator<_Tp> &operator*() { return *this; }

  array_back_insert_iterator<_Tp> &operator++() { return *this; }

  array_back_insert_iterator<_Tp> &operator++(int) { return *this; }
};

/**
 * convenience function template that constructs a array_back_insert_iterator
 * for the C array with the type deduced from the type of the argument.
 *
 * @array: begining of array
 * @size: max number element to insert
 */
template <class _Tp>
array_back_insert_iterator<_Tp> array_back_inserter(_Tp *array, size_t size) {
  return array_back_insert_iterator<_Tp>(array, size);
}

#endif // _UTILS_BACK_INSERT_ITERATOR_H_
