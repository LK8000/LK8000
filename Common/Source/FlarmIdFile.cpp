/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "externs.h"
#include "FlarmIdFile.h"
#include "utils/array_back_insert_iterator.h"
#include "utils/zzip_stream.h"
#include <iostream>

namespace {

std::string::const_iterator GetAsString(std::string::const_iterator it, size_t size, TCHAR *res) {

  auto out = array_back_inserter(res, size - 1); // size - 1 to let placeholder for '\0'
  for(unsigned i = 0; i < (size -1); ++i) {
    out = (HexDigit(*(it++)) << 4) | HexDigit(*(it++));
  }
  *out = _T('\0');

  // remove trailing whitespace
  TCHAR* end = res + size - 1;
  while ((--end) >= res && _istspace(*end)) {
    *end = _T('\0');
  }
  return it;
}

} // namespace

FlarmId::FlarmId(const std::string& string) {
  if(string.length() != 172) {
    throw std::runtime_error("invalid flarmnet record");
  }

  auto it = string.begin();
  it = GetAsString(it, FLARMID_SIZE_ID, id);
  it = GetAsString(it, FLARMID_SIZE_NAME, name);
  it = GetAsString(it, FLARMID_SIZE_AIRFIELD, airfield);
  it = GetAsString(it, FLARMID_SIZE_TYPE, type);
  it = GetAsString(it, FLARMID_SIZE_REG, reg);
  it = GetAsString(it, FLARMID_SIZE_CN, cn);
  it = GetAsString(it, FLARMID_SIZE_FREQ, freq);


  // Add a valid CN if missing. Ex: D-6543 = D43
  if (_tcslen(cn) == 0 ) {
    int reglen=_tcslen(reg);
    if (reglen >=3) {
      cn[0] = reg[0];
      cn[1] = reg[reglen-2];
      cn[2] = reg[reglen-1];
      cn[3] = _T('\0');
    }
  }
}



FlarmIdFile::FlarmIdFile() {

  TCHAR flarmIdFileName[MAX_PATH] = _T("");
  LocalPath(flarmIdFileName, _T(LKD_CONF), _T(LKF_FLARMNET));

  /*
   * we can't use std::ifstream due to lack of unicode file name in mingw32
   */
  zzip_stream file(flarmIdFileName, "rt");
  if (!file) {
    LocalPath(flarmIdFileName, _T(LKD_CONF), _T("data.fln"));
    file.open(flarmIdFileName, "rt");
  }
  if (!file) {
    return;
  }


  std::string src_line;
  src_line.reserve(173);

  std::istream stream(&file);
  std::getline(stream, src_line); // skip first line
  while (std::getline(stream, src_line)) {
    try {
      FlarmId *flarmId = new FlarmId(src_line);
      auto ib = flarmIds.insert(std::make_pair(flarmId->GetId(), flarmId));
      if (!ib.second) {
        assert(false); // duplicated id ! invalid file ?
        delete flarmId;
      }
    } catch (std::runtime_error& e) {
      StartupStore(_T("%s"), to_tstring(e.what()).c_str());
    }
  }
}

FlarmIdFile::~FlarmIdFile() {
  flarmIds.clear();
}

const FlarmId* FlarmIdFile::GetFlarmIdItem(uint32_t id) const {
  auto it = flarmIds.find(id);
  if (it != flarmIds.end()) {
    return it->second.get();
  }
  return nullptr;
}

const FlarmId* FlarmIdFile::GetFlarmIdItem(const TCHAR *cn) const {
  auto it = std::find_if(std::begin(flarmIds), std::end(flarmIds), [&](auto& item) {
    return (_tcscmp(item.second->cn, cn) == 0);
  });

  if (it != flarmIds.end()) {
    return it->second.get();
  }
  return nullptr;
}

uint32_t FlarmId::GetId() const {
  return _tcstoul(id, nullptr, 16);
}
