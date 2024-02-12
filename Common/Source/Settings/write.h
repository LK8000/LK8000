/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#ifndef _SETTINGS_WRITE_H_
#define _SETTINGS_WRITE_H_

#include "Compiler.h"
#include "Defines.h"
#include "tchar.h"
#include <type_traits>
#include <stdio.h>

#define PNEWLINE  "\r\n"

namespace settings {

  class writer final {
  public:
    writer() = delete;

    writer(const TCHAR* szFile, const char* name) {
      // 'w' will overwrite content, 'b' for no crlf translation
      file = _tfopen(szFile, _T("wb"));
      if(file) {
        fprintf(file, "### LK8000 %s - DO NOT EDIT" PNEWLINE, name);
        fprintf(file, "### THIS FILE IS ENCODED IN UTF8" PNEWLINE);
        fprintf(file, R"(LKVERSION=")" LKVERSION "." LKRELEASE R"(")" PNEWLINE);
        fprintf(file, "PROFILEVERSION=2" PNEWLINE);
      }
    }

    ~writer() {
      if(file) {
        fprintf(file, PNEWLINE); // end of file
        fflush(file);
        fclose(file);
      }
    }

    /**
     * @return true if file is open.
     */
    operator bool() {
      return (file != nullptr);
    }

    template<typename T>
    void operator()(const char *name, T value) {
      write_value<T>(name, value);
    }

  private:

    /**
     * generic #write_value for all signed integer
     */
    template<typename T>
    using signed_integer =
            std::enable_if_t<(std::is_integral_v<T> && std::is_signed_v<T> && !std::is_enum_v<T>), T>;

    template<typename T>
    void write_value(const char *name, signed_integer<T> value) {
      fprintf(file, "%s=%d" PNEWLINE, name, static_cast<int>(value));
    }

    /**
     * generic #write_value for all unsigned integral or enum type
     */
    template<typename T>
    using unsigned_integer =
            std::enable_if_t<(std::is_integral_v<T> && !std::is_signed_v<T>) || std::is_enum_v<T>, T>;

    template<typename T>
    void write_value(const char *name, unsigned_integer<T> value) {
      fprintf(file, "%s=%u" PNEWLINE, name, static_cast<unsigned int>(value));
    }

    /**
     * generic #write_value for all floating point
     *  tips : floating point is saved without decimal point ( like int )
     *     but we use printf %.0f instead of %d and cast to int for rounding...
     */
    template<typename T>
    using floating_point =
            std::enable_if_t<std::is_floating_point_v<T>, T>;

    template<typename T>
    void write_value(const char *name, floating_point<T> value) {
      fprintf(file, "%s=%.0f" PNEWLINE, name, static_cast<double>(value));
    }

    /**
     * template specialization for utf8 string (aka const char*)
     */
    template<typename T>
    using char_string =
            std::enable_if_t<std::is_same_v<std::remove_cv_t<T>, char*>>;

    template<typename T, typename = char_string<T>>
    void write_value(const char *name, const char* value) {
      fprintf(file, "%s=\"%s\"" PNEWLINE, name, value);
    }

    /**
     * template specialization for utf8 string (aka const std::string&)
     */
    template<typename T, typename = std::enable_if_t<std::is_same_v<T, std::string>>>
    void write_value(const char *name, const std::string& value) {
      write_value<char*>(name, value.c_str());
    }

#ifdef _UNICODE
    /**
     * unicode string
     */
    template<typename T>
    using wchar_string =
            std::enable_if_t<std::is_same_v<std::remove_cv_t<T>, wchar_t*>>;

    /**
     * template specialization for unicode string (aka const wchar_t*)
     */
    template<typename T, typename = wchar_string<T>>
    void write_value(const char *name, const wchar_t* value) {
      write_value<std::string>(name, to_utf8(value));
    }
#endif

  private:
    FILE *file;
  };

} // settings

#endif // _SETTINGS_WRITE_H_
