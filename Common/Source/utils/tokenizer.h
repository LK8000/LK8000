/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   tokenizer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 août 2017
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <algorithm>
#include <cassert>
#include <iterator>

namespace lk {
 /**
  * tokenize string like strtok or strsep
  * 
  * this helper modify the input string, so :
  *  - can't be used to tokenize a const strings
  *  - input string is modified ofter first "next(...) call
  * 
  * Each call to next() method returns a pointer to a null-terminated string containing the next token. 
  * This string does not include the delimiting byte. If no more tokens are found, next() method returns nullptr.
  */
  template<typename CharT>
  class tokenizer final {
  public:
    tokenizer() = delete;

    explicit tokenizer(CharT* szString) : m_szBegin(szString), m_szEnd(szString) {
      assert(m_szBegin && m_szEnd);
      while (m_szEnd && *m_szEnd) {
        ++m_szEnd;
      }
    }

    /**
     * Each call returns a pointer to a null-terminated string containing the next token. 
     * This string does not include the delimiting byte. If no more tokens are found, this method returns nullptr.
     * 
     * @delim is array of CharT and not a string, so to avoid bug on utf8 platform use only 7bit Ascii.
     */
    template<size_t N>
    CharT* Next(const CharT(&delim)[N], bool skip_leading_delimiter = false) {

      if (skip_leading_delimiter) {
        m_szBegin = std::find_if_not(m_szBegin, m_szEnd, IsDelimiter<N>(delim));
      }
      if(m_szBegin != m_szEnd) {
        CharT* szOut = m_szBegin;
        m_szBegin = std::find_if(m_szBegin, m_szEnd, IsDelimiter<N>(delim));
        if (m_szBegin != m_szEnd) {
          *(m_szBegin++) = 0; // replace separator by '\0' and advance to next token.
        }
        return szOut;
      }
      return nullptr;
    }

    /**
     * to get the remaining part of input string
     */
    CharT* Remaining() const {
      return m_szBegin;
    }

  private:

    template<size_t N>
    class IsDelimiter final {
    public:
      IsDelimiter() = delete;

      explicit IsDelimiter(const CharT(&sep)[N]) : _sep(sep) { }

      bool operator()(const CharT& c) const {
        return std::find(std::begin(_sep), std::end(_sep), c) != std::end(_sep);
      }
    private:
      const CharT(&_sep)[N];
    };

    CharT* m_szBegin;
    CharT* m_szEnd;
  };

} // namespace lk

#endif /* TOKENIZER_H */
