/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "externs.h"
#include "FlarmIdFile.h"
#include "utils/array_back_insert_iterator.h"
#include "utils/zzip_stream.h"
#include "utils/charset_helper.h"
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

void ExtractParameter(const TCHAR* Source, TCHAR* Destination, int DesiredFieldNumber) {
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength = _tcslen(Source);
  const TCHAR* sptr = Source;
  const TCHAR* eptr = Source + StringLength;

  if (!Destination)
    return;

  while ((CurrentFieldNumber < DesiredFieldNumber) && (sptr < eptr)) {
    if (*sptr == ',') {
      CurrentFieldNumber++;
    }
    ++sptr;
  }

  Destination[0] = '\0';  // set to blank in case it's not found..

  if (CurrentFieldNumber == DesiredFieldNumber) {
    while ((sptr < eptr) && (*sptr != ',') && (*sptr != '\0')) {
      Destination[dest_index] = *sptr;
      ++sptr;
      if (Destination[dest_index] != '\'')  // remove '
        ++dest_index;
    }
    Destination[dest_index] = '\0';
  }
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


void FlarmIdFile::LoadOgnDb() {

  TCHAR OGNIdFileName[MAX_PATH] = _T("");
  LocalPath(OGNIdFileName, _T(LKD_CONF), _T("data.ogn"));

  /*
   * we can't use std::ifstream due to lack of unicode file name in mingw32
   */
  zzip_stream file(OGNIdFileName, "rt");
  if (!file) {
    return;
  }


  std::string src_line;
  src_line.reserve(512);
  unsigned int Doublicates = 0;
  unsigned int InvalidIDs = 0;
  std::istream stream(&file);
  while (std::getline(stream, src_line)) {
    try {

      if (src_line.empty() || src_line.front() == '#') {
        continue; // skip empty line and comments
      }

      tstring t_line = from_unknown_charset(src_line.c_str());

      TCHAR id[7] = {};
      ExtractParameter(t_line.c_str(), id, 1);
      uint32_t RadioId = _tcstoul(id, nullptr, 16);

      auto ib = flarmIds.emplace(RadioId, nullptr);
      if (ib.second) {
        // new item instead ?
        auto flarmId = std::make_unique<FlarmId>();

        _tcscpy(flarmId->id, id);
        ExtractParameter(t_line.c_str(), flarmId->reg, 3);
        if (_tcslen(flarmId->reg) == 0) {
          // reg empty use id...
          _stprintf(flarmId->reg, _T("%X"), RadioId);
          InvalidIDs++;
        }

        ExtractParameter(t_line.c_str(), flarmId->type, 2);
        _stprintf(flarmId->name, _T("OGN: %X"), RadioId);
        ExtractParameter(t_line.c_str(), flarmId->cn, 4);

        DebugLog(_T("===="));
        DebugLog(_T("OGN %s"), t_line.c_str());
        DebugLog(_T("OGN ID=%s"), flarmId->id);
        DebugLog(_T("OGN Type=%s"), flarmId->type);
        DebugLog(_T("OGN Name=%s"), flarmId->name);
        DebugLog(_T("OGN CN=%s"), flarmId->cn);

        ib.first->second = std::move(flarmId);  // transfert flarmId ownership to 'flarmIds' map.
      } else {
        Doublicates++;
      }

    } catch (std::exception& e) {
      StartupStore(_T("%s"), to_tstring(e.what()).c_str());
    }
  }
  if (InvalidIDs > 0)
    StartupStore(_T(". found %u invalid IDs in OGN database"),InvalidIDs);	
  if (Doublicates > 0)
    StartupStore(_T(". found %u IDs also in OGN database -> ignored"),Doublicates);
}


void FlarmIdFile::LoadFlarmnetDb() {

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
  if (file) {
    std::string src_line;
    src_line.reserve(173);

    std::istream stream(&file);
    std::getline(stream, src_line); // skip first line
    while (std::getline(stream, src_line)) {
      try {
        auto flarmId = std::make_unique<FlarmId>(src_line);
        auto ib = flarmIds.emplace(flarmId->GetId(), std::move(flarmId));
        assert(ib.second); // duplicated id ! invalid file ?
      } catch (std::exception& e) {
        StartupStore(_T("%s"), to_tstring(e.what()).c_str());
      }
    }
  }
}

FlarmIdFile::FlarmIdFile() {

  LoadFlarmnetDb();
  auto FlamnetCnt = static_cast<unsigned>(flarmIds.size());
  StartupStore(_T(". FLARMNET database, found %u IDs"), FlamnetCnt);

  LoadOgnDb();
  auto OgnCnt = static_cast<unsigned>(flarmIds.size() - FlamnetCnt);
  StartupStore(_T(". OGN database, found additinal %u IDs"), OgnCnt);

  StartupStore(_T(". total %u Flarm device IDs found!"), static_cast<unsigned>(flarmIds.size()));
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
    return (item.second && _tcscmp(item.second->cn, cn) == 0);
  });

  if (it != flarmIds.end()) {
    return it->second.get();
  }
  return nullptr;
}

uint32_t FlarmId::GetId() const {
  return _tcstoul(id, nullptr, 16);
}
