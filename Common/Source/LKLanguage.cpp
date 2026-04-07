/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: LKLanguage.cpp,v 1.4 2010/12/20 23:35:24 root Exp root $
 */

#include "externs.h"
#include "utils/stl_utils.h"
#include "utils/zzip_file_stream.h"
#include <nlohmann/json.hpp>
#include <istream>
#include "LocalPath.h"
#ifdef ANDROID
  #include "Android/Main.hpp"
  #include "Android/NativeView.hpp"
#endif

using json = nlohmann::json;

extern void FillDataOptions();

LKLanguages_t LKMessages = {};

namespace {

  constexpr auto InvalidTextIndex = std::numeric_limits<unsigned>::max();

  template<typename CharT>
  unsigned GetTextIndex(const CharT *key, size_t size, char type) {
    if ((size >= 5) && (key[0] == '_') && (key[1] == '@')
        && (key[2] == type) && (key[size - 1] == '_')) {

      // get the item index number
      unsigned index = 0;
      for (unsigned int i = 3; i < size - 1; i++) {
        if (!isdigit(key[i])) {
          return InvalidTextIndex;
        }
        index = (index * 10U) + (key[i] - '0');
      }
      return index;
    }
    return InvalidTextIndex;
  }

  unsigned GetTextIndex(const TCHAR *key, char type) {
    return key
        ? GetTextIndex(key, _tcslen(key), type)
        : InvalidTextIndex;
  }

  unsigned GetTextIndex(const std::string &key, char type) {
    return GetTextIndex(key.c_str(), key.size(), type);
  }

  void LKLoadMessages(const json& lang_json) {
    if (!lang_json.is_object()) {
      return;
    }
    for (const auto& obj : lang_json.items()) {
      // get the item index number
      const unsigned index = GetTextIndex(obj.key(), 'M');
      if (index < std::size(LKMessages)) {
        if (!LKMessages[index] && obj.value().is_string()) {
          const tstring value = utf8_to_tstring(obj.value());
          LKMessages[index] = _tcsdup(value.c_str());
        }
      }
      else {
        // invalid token or LKMessages array too small.
        assert(index == InvalidTextIndex);
      }
    }
  }

  void LoadLanguageList(const TCHAR* szFilePath, std::map<tstring, tstring>& language) {
    zzip_file_stream file(szFilePath, "rb");
    if (file) {
      try {
        std::istream stream(&file);
        json lang_json = json::parse(stream);
        for (const auto& obj : lang_json.items()) {
          if (obj.value().is_string()) {
            language.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(utf8_to_tstring(obj.key())),
                std::forward_as_tuple(utf8_to_tstring(obj.value())));
          }
        }
      }
      catch (std::exception& e) {
        StartupStore(_T("language : %s <%s>"), szFilePath, to_tstring(e.what()).c_str());
      }
    }
  }

  json GetLanguageJson(const TCHAR* code) {
    json json_lang;
    try {
      TCHAR szFileName[MAX_PATH] = {};
      TCHAR szFilePath[MAX_PATH] = {};
      lk::snprintf(szFileName, _T("%s.json"), code);
      LocalPath(szFilePath, _T(LKD_LANGUAGE), szFileName);

      zzip_file_stream file(szFilePath, "rb");
      if (!file) {
        SystemPath(szFilePath, _T(LKD_SYS_LANGUAGE), szFileName);
        file = zzip_file_stream(szFilePath, "rb");
      }
      if(file) {
        std::istream stream(&file);
        json_lang = json::parse(stream);
      } else {
        StartupStore(_T("... Missing Language FILE <%s>"), szFilePath);
      }
    } catch(json::parse_error& error) {
      StartupStore(_T("language : %s"), to_tstring(error.what()).c_str());
    }
    return json_lang;
  }

} // namespace


// _@Hnnnn_
// minimal: _@H1_  maximal: _@H999999_
tstring LKgethelptext(const TCHAR *TextIn) {
  const unsigned index = GetTextIndex(TextIn, 'H');
  if (index > 999999) {
    return TextIn;
  }

  char sToken[11];
  sprintf(sToken, "_@H%06u_", index);

  tstring sHelpString;
  try {
    json lang_json = GetLanguageJson(szLanguageCode);
    auto string = lang_json.find(sToken);
    if(string == lang_json.end()) {
      // token not found in user language, try system language
      lang_json = GetLanguageJson(_T(LKD_DEFAULT_LANGUAGE));
      string = lang_json.find(sToken);
    }
    if (string != lang_json.end()) {
      const json& value = string.value();
      if (value.is_string()) {
        sHelpString = utf8_to_tstring(value);
      } else {
        sHelpString = TextIn;
      }
    } else {
      StartupStore(_T(".... Unknown Text token <%s>"), TextIn);
      sHelpString = TextIn;
    }
  } catch (json::exception& error) {
    StartupStore(_T("language : %s"), to_tstring(error.what()).c_str());
    sHelpString = TextIn;
  }
  return sHelpString;
}

///  Tokenized Language support for LK8000
// token must be "_@Mxxxx_" with xxxx a valid number.
// return TextIn if token is not valid
const TCHAR *LKGetText(const TCHAR *TextIn) {
  const unsigned index = GetTextIndex(TextIn, 'M');
  if (index < std::size(LKMessages) && LKMessages[index]) {
    return LKMessages[index];
  }
  return TextIn;
}

void LKLoadLanguageFile() {

  LKUnloadLanguageFile();

#ifdef ANDROID
  if (szLanguageCode[0] == _T('\0')) {
    // use device language
    if(native_view) {
      const char* default_language = native_view->GetDefaultLanguage();
      if(default_language) {
        lk::strcpy(szLanguageCode, default_language);
      }
    }
  }
#endif

  if (szLanguageCode[0] == _T('\0')) {
    lk::strcpy(szLanguageCode, _T(LKD_DEFAULT_LANGUAGE));
  }

  LKLoadMessages(GetLanguageJson(szLanguageCode));

  if (_tcscmp(szLanguageCode, _T(LKD_DEFAULT_LANGUAGE)) != 0) {
    // fill up with default language
    LKLoadMessages(GetLanguageJson(_T(LKD_DEFAULT_LANGUAGE)));
  }

  FillDataOptions(); // Load infobox list
}

void LKUnloadLanguageFile() {
  std::for_each(std::begin(LKMessages), std::end(LKMessages), safe_free());
}

std::map<tstring, tstring> LoadLanguageList() {

  std::map<tstring, tstring> language;
  TCHAR szFilePath[MAX_PATH] = {};

  // user language list
  LocalPath(szFilePath, _T(LKD_LANGUAGE), _T("language.json"));
  LoadLanguageList(szFilePath, language);

  if constexpr (std::string_view(LKD_LANGUAGE) !=
                std::string_view(LKD_SYS_LANGUAGE)) {
    // fill up with system language
    SystemPath(szFilePath, _T(LKD_SYS_LANGUAGE), _T("language.json"));
    LoadLanguageList(szFilePath, language);
  }
  return language;
}
