/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: LKLanguage.cpp,v 1.4 2010/12/20 23:35:24 root Exp root $
 */

#include "externs.h"
#include "utils/stl_utils.h"
#include "utils/zzip_stream.h"
#include "picojson.h"
#include <istream>
#ifdef ANDROID
  #include "Android/Main.hpp"
  #include "Android/NativeView.hpp"
#endif

namespace json = picojson;

extern void FillDataOptions();

namespace {

  constexpr size_t MAX_MESSAGES = 2500; // Max number of MSG items
  TCHAR *LKMessages[MAX_MESSAGES] = {};

  template<typename CharT>
  unsigned GetTextIndex(const CharT *key, size_t size, char type) {
    if ((size >= 5) && (key[0] == '_') && (key[1] == '@')
        && (key[2] == type) && (key[size - 1] == '_')) {

      // get the item index number
      unsigned index = 0;
      for (unsigned int i = 3; i < size - 1; i++) {
        if (!isdigit(key[i])) {
          // invalid token
          return std::numeric_limits<unsigned>::max();
        }
        index = (index * 10U) + (key[i] - '0');
      }
      return index;
    }
    return std::numeric_limits<unsigned>::max();
  }

  unsigned GetTextIndex(const TCHAR *key, char type) {
    return (key
        ? GetTextIndex(key, _tcslen(key), type)
        : std::numeric_limits<unsigned>::max());
  }

  unsigned GetTextIndex(const std::string &key, char type) {
    return GetTextIndex(key.c_str(), key.size(), type);
  }

  void LKLoadMessages(const json::value &lang_json) {
    if(lang_json.is<json::object>()) {
      for (const auto &obj : lang_json.get<json::object>()) {
        // get the item index number
        const unsigned index = GetTextIndex(obj.first, 'M');
        if (index < array_size(LKMessages) && !LKMessages[index]) {
          if(obj.second.is<std::string>()) {
            const tstring value = utf8_to_tstring(obj.second.get<std::string>().c_str());
            LKMessages[index] = _tcsdup(value.c_str());
          }
        }
      }
    }
  }

  void LoadLanguageList(const TCHAR* szFilePath, std::map<tstring, tstring>& language) {
    zzip_stream file(szFilePath, "rb");
    if (file) {
      std::istream stream(&file);
      json::value lang_json;
      std::string error = json::parse(lang_json, stream);
      if(error.empty()) {
        if(lang_json.is<json::object>()) {
          for (const auto &obj : lang_json.get<json::object>()) {
            if(obj.second.is<std::string>()) {
              const tstring code = utf8_to_tstring(obj.first.c_str());
              const tstring name = utf8_to_tstring(obj.second.get<std::string>().c_str());

              /*
               * would be better to use std::map::emplace() instead of insert() to
               * avoid intermediate pair construction, but "emplace" does not exist
               * in gcc 4.6.3 ( used for WinCE target )
               *
               *  language.emplace(std::piecewise_construct,
               *                   std::forward_as_tuple(code),
               *                   std::forward_as_tuple(name));
               */

              language.insert(std::make_pair(code, name));
            }
          }
        } else {
          StartupStore(_T("language : %s <%s>"), szFilePath , to_tstring(lang_json.to_str().c_str()).c_str());
        }
      } else {
        StartupStore(_T("language : %s"), to_tstring(error.c_str()).c_str());
      }
    }
  }

  json::value GetLanguageJson(const TCHAR* code) {
    json::value json_lang;

    TCHAR szFileName[MAX_PATH] = {};
    TCHAR szFilePath[MAX_PATH] = {};
    _stprintf(szFileName, _T("%s.json"), code);
    LocalPath(szFilePath, _T(LKD_LANGUAGE), szFileName);

    zzip_stream file(szFilePath, "rb");
    if (!file) {
      SystemPath(szFilePath, _T(LKD_SYS_LANGUAGE), szFileName);
      file.open(szFilePath, "rb");
    }
    if(file) {
      std::istream stream(&file);
      std::string error = json::parse(json_lang, stream);
      if(!error.empty()) {
        StartupStore(_T("language : %s"), to_tstring(error.c_str()).c_str());
      }
    } else {
      StartupStore(_T("... Missing Language FILE <%s>"), szFilePath);
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

  json::value lang_json = GetLanguageJson(szLanguageCode);
  auto string = lang_json.get(sToken);
  if(string.is<json::null>()) {
    // token not found in user language, try system language
    lang_json = GetLanguageJson(_T(LKD_DEFAULT_LANGUAGE));
    string = lang_json.get(sToken);
  }
  if (string.is<std::string>()) {
    sHelpString = utf8_to_tstring(string.get<std::string>().c_str());
  } else {
    StartupStore(_T(".... Unknown Text token <%s>"), TextIn);
    sHelpString = TextIn;
  }

  return sHelpString;
}

///  Tokenized Language support for LK8000
// token must be "_@Mxxxx_" with xxxx a valid number.
// return TextIn if token is not valid
const TCHAR *LKGetText(const TCHAR *TextIn) {
  const unsigned index = GetTextIndex(TextIn, 'M');
  if (index < array_size(LKMessages) && LKMessages[index]) {
    return LKMessages[index];
  }
  return TextIn;
}

/// Direct token access, with range check, faster than LKGetText
// return empty string if token is not found
const TCHAR *MsgToken(unsigned index) {
  if (index < array_size(LKMessages) && LKMessages[index]) {
    return LKMessages[index];
  }
  return _T("");
}

void LKReadLanguageFile() {

  LKUnloadMessage();

#ifdef ANDROID
  if (szLanguageCode[0] == _T('\0')) {
    // use device language
    if(native_view) {
      const char* default_language = native_view->GetDefaultLanguage();
      if(default_language) {
        _tcscpy(szLanguageCode, default_language);
      }
    }
  }
#endif

  if (szLanguageCode[0] == _T('\0')) {
    _tcscpy(szLanguageCode, _T(LKD_DEFAULT_LANGUAGE));
  }

  LKLoadMessages(GetLanguageJson(szLanguageCode));

  if (_tcscmp(szLanguageCode, _T(LKD_DEFAULT_LANGUAGE)) != 0) {
    // fill up with default language
    LKLoadMessages(GetLanguageJson(_T(LKD_DEFAULT_LANGUAGE)));
  }

  FillDataOptions(); // Load infobox list
}

void LKUnloadMessage() {
  std::for_each(std::begin(LKMessages), std::end(LKMessages), safe_free());
}

std::map<tstring, tstring> LoadLanguageList() {

  std::map<tstring, tstring> language;
  TCHAR szFilePath[MAX_PATH] = {};

  // user language list
  LocalPath(szFilePath, _T(LKD_LANGUAGE), _T("language.json"));
  LoadLanguageList(szFilePath, language);

  // fill up with system language
  SystemPath(szFilePath, _T(LKD_SYS_LANGUAGE), _T("language.json"));
  LoadLanguageList(szFilePath, language);

  return language;
}
