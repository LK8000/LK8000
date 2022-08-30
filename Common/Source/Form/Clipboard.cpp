/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Clipboard.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 10, 2024
 */
#include <exception>
#include "Clipboard.h"
#include "tchar.h"
#include "MessageLog.h"
#ifdef ANDROID
  #include "Android/LK8000Activity.h"
#endif

#if defined(WIN32) && !defined(PPC2003)
#include "windows.h"

namespace {

struct ScopeClipboard {
  ScopeClipboard() {
    if (!OpenClipboard(NULL)) {
      throw std::runtime_error("failed to open clipboard");
    }
  }

  ~ScopeClipboard() {
    CloseClipboard();
  }
};

template<typename TypeT>
class ScopeGlobalLock {
public:
  ScopeGlobalLock(HANDLE hdata) : _hdata(hdata) {
    _pdata = static_cast<const TypeT*>(::GlobalLock(hdata));
    if (!_pdata) {
      throw std::runtime_error("failed to lock clipboard data");
    }
  }

  ~ScopeGlobalLock() {
    ::GlobalUnlock(_hdata);
  }

  operator const TypeT*() const {
    return _pdata;
  }

private:
  HANDLE _hdata;
  const TypeT* _pdata;
};

} // namespace

#endif

bool ClipboardAvailable() {
#if defined(ANDROID) || !defined(NDEBUG) || (defined(WIN32) && !defined(PPC2003))
  return true;
#endif
  return false;
}


tstring GetClipboardData() {

#ifdef ANDROID
  LK8000Activity* activity = LK8000Activity::Get();
  if (activity) {
    return activity->GetClipboardText();
  }
#endif

#if defined(WIN32) && !defined(PPC2003)
  try {
    ScopeClipboard clipboard;
    const wchar_t* data = ScopeGlobalLock<wchar_t>(::GetClipboardData(CF_UNICODETEXT));
    return data;
  }
  catch(std::exception& e) {
    StartupStore(_T("%s"), to_tstring(e.what()).c_str());
  }
#endif

  return _T("");
}
