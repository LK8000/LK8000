/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   DeviceSettings.h
 *  Author: Bruno de Lacheisserie
 */

#include "DeviceSettings.h"
#include "externs.h"
#include "Utils.h"
#include <fstream>

namespace {

tstring get_file_path(const TCHAR* name) {
  TCHAR szFilePath[MAX_PATH] = {};
  LocalPath(szFilePath, _T(LKD_CONF), name);
  _tcscat(szFilePath, _T(".json"));
  return szFilePath;
}

} // namespace


DeviceSettings::DeviceSettings(const TCHAR* name) : file_path(get_file_path(name)) {
  try {
    std::ifstream file_in(file_path.c_str());
    file_in >> data;
  }
  catch (std::exception& e) {
    StartupStore(_T("failed to load <%s> : %s"), file_path.c_str(), to_tstring(e.what()).c_str());
  }
  if (!data.is<picojson::object>()) {
    data.set(picojson::object());
  }
}

DeviceSettings::~DeviceSettings() {
  Commit();
}

void DeviceSettings::Commit() {
  try {
    std::ofstream file_out(file_path.c_str());
    data.serialize(std::ostream_iterator<char>(file_out), true);
  }
  catch (std::exception& e) {
    StartupStore(_T("failed to save <%s> : %s"), file_path.c_str(), to_tstring(e.what()).c_str());
  }
}
