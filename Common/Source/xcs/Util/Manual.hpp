/*
 * Copyright (C) 2013 Max Kellermann <max@duempel.org>
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

#ifndef MANUAL_HPP
#define MANUAL_HPP

#include "Compiler.h"

#include <new>
#include <type_traits>
#include <memory>
#include <utility>
#include <cassert>


/**
 * Container for an object that gets constructed and destructed
 * manually.  The object is constructed in-place, and therefore
 * without allocation overhead.  It can be constructed and destructed
 * repeatedly.
 */
template<class T>
class Manual {

  alignas(T) std::byte data[sizeof(T)];

  
  T* addr() noexcept {
    return std::launder(reinterpret_cast<T*>(&data));
  }
  const T* addr() const noexcept {
    return std::launder(reinterpret_cast<const T*>(&data));
  }

#ifndef NDEBUG
  bool initialized = false;
#endif

public:

  Manual() = default;

  Manual(const Manual&) = delete;
  Manual& operator=(const Manual&) = delete;
  Manual(Manual&&) = delete;
  Manual& operator=(Manual&&) = delete;

#ifndef NDEBUG
  ~Manual() {
    assert(!initialized);
  }
#endif

  template<typename... Args>
  void Construct(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))){
    assert(!initialized);

    std::construct_at(reinterpret_cast<T*>(&data), std::forward<Args>(args)...);

#ifndef NDEBUG
    initialized = true;
#endif
  }

  void Destruct() noexcept {
    assert(initialized);

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(addr());
    }

#ifndef NDEBUG
    initialized = false;
#endif
  }

  T &Get() noexcept {
    assert(initialized);
    return *addr();
  }

  const T &Get() const noexcept {
    assert(initialized);
    return *addr();
  }

  operator T &() noexcept {
    return Get();
  }

  operator const T &() const noexcept {
    return Get();
  }

  T *operator->() noexcept {
    return &Get();
  }

  const T *operator->() const noexcept {
    return &Get();
  }
};

#endif
