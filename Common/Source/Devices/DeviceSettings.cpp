#include "DeviceSettings.h"
#include "externs.h"
#include "Utils.h"
#include <fstream>

namespace {

tstring get_file_path(const TCHAR* name) {
  TCHAR szFilePath[MAX_PATH] = {};
  LocalPath(szFilePath, _T(LKD_CONF), name);
  _tcscat(szFilePath, ".json");
  return szFilePath;
}

} // namespace


DeviceSettings::DeviceSettings(const TCHAR* name) : file_path(get_file_path(name)) {
  try {
    std::ifstream file_in(file_path, std::ifstream::in);
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
    std::ofstream file_out(file_path);
    data.serialize(std::ostream_iterator<char>(file_out), true);
  }
  catch (std::exception& e) {
    StartupStore(_T("failed to save <%s> : %s"), file_path.c_str(), to_tstring(e.what()).c_str());
  }
}
