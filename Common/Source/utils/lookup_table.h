/* Copyright (c) 2020, Bruno de Lacheisserie
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

#ifndef _utils_lookup_table_h_
#define _utils_lookup_table_h_

#include <cstddef>
#include <utility>
#include <functional>

template<typename key_type, typename mapped_type, size_t size, typename key_comp>
class lookup_table_t {
private:
  typedef std::pair<const key_type, const mapped_type> value_type; 

  const value_type _data[size];

public:
  template<size_t... I>
  constexpr lookup_table_t(const value_type (&data)[size], std::index_sequence<I...>) noexcept
      : _data{{data[I].first, data[I].second}...} {
  }

  /**
   * return @def_value if @key is not inside table.
   */
  inline constexpr
  mapped_type get(key_type key, const mapped_type &def_value) const {
    key_comp compare;
    for (const auto &item : _data) {
      if (compare(item.first, key)) {
        return item.second;
      }
    }
    return def_value;
  }

  /**
   * if @key_type and @mapped_type as same type, return @key if @key is not inside table.
   */
  template<typename T = mapped_type,
           typename=std::enable_if_t<std::is_same_v<key_type, T>>>
  inline constexpr
  T get(key_type key) const {
    return get(key, key);
  }
};

/**
 * convenience function template that constructs a lookup_table_t 
 * with size deduced from argument
 */
template<typename key_type, typename mapped_type, size_t size>
constexpr lookup_table_t<key_type, mapped_type, size, std::equal_to<key_type>>
lookup_table(const std::pair<const key_type, const mapped_type> (&data)[size]) noexcept {
  return lookup_table_t<key_type, mapped_type, size, std::equal_to<key_type>>(data, std::make_index_sequence<size>());
}

/**
 * convenience function template that constructs a lookup_table_t 
 * with size deduced from argument and custom key compare predicate
 */
template<typename key_type, typename mapped_type, typename key_comp, size_t size>
constexpr lookup_table_t<key_type, mapped_type, size, key_comp>
lookup_table(const std::pair<const key_type, const mapped_type> (&data)[size]) noexcept {
  return lookup_table_t<key_type, mapped_type, size, key_comp>(data, std::make_index_sequence<size>());
}

#endif // _utils_lookup_table_h_
