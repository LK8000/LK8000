/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "externs.h"
#include "FlarmIdFile.h"
#include "utils/zzip_stream.h"
#include <unordered_map>
#include <memory>

namespace {

  std::unordered_map<uint32_t, tstring> map_flarm_names;
  std::unique_ptr<FlarmIdFile> flarmnet_database;

  void SaveFLARMDetails() {
    TCHAR filename[MAX_PATH];
    LocalPath(filename,TEXT(LKD_CONF),_T(LKF_FLARMIDS)); // 091103

    FILE * stream = _tfopen(filename,_T("wt"));
    if(stream) {
      for (const auto& item : map_flarm_names) {
        _ftprintf(stream, _T("%x=%s\n"), item.first,item.second.c_str());
      }
      fclose(stream);

      StartupStore(_T("... Saved %u FLARM names"), (unsigned)map_flarm_names.size());
    } else {
      StartupStore(_T("-- Cannot save FLARM details, error --\n"));
    }
  }

} // namespace


void CloseFLARMDetails() {
  flarmnet_database = nullptr;
  map_flarm_names.clear();
}


void OpenFLARMDetails() {

  flarmnet_database = std::make_unique<FlarmIdFile>();

  StartupStore(_T(". FLARMNET database, found %u IDs"), (unsigned)flarmnet_database->Count());

  TCHAR filename[MAX_PATH];
  LocalPath(filename,TEXT(LKD_CONF),_T(LKF_FLARMIDS));
  TestLog(_T("... OpenFLARMDetails: <%s>"),filename);

  zzip_stream stream(filename, "rt");
  if( !stream ) {
    TestLog(_T("... No flarm details local file found"));
    return;
  }

  TCHAR line[READLINE_LENGTH];
  while (stream.read_line(line)) {
    TCHAR* next = nullptr;
    uint32_t RadioId = _tcstoul(line, &next, 16);
    if ( next && (*next) == _T('=')) {
      AddFlarmLookupItem(RadioId, ++next, false);
    }
  }

  if (!map_flarm_names.empty()) {
    StartupStore(_T(". Local IDFLARM, found %u IDs"), (unsigned)map_flarm_names.size());
  }
}


// returns Name or Cn to be used
const TCHAR* LookupFLARMCn(uint32_t RadioId) {
  // try to find flarm from userFile
  auto it = map_flarm_names.find(RadioId);
  if(it != map_flarm_names.end()) {
    return it->second.c_str();
  }

  // try to find flarm from FLARMNet.org File
  if (flarmnet_database) {
    const FlarmId* flarmId = flarmnet_database->GetFlarmIdItem(RadioId);
    if (flarmId) {
      return flarmId->cn;
    }
  }
  return nullptr;
}

// returns Glider type if available
const FlarmId* LookupFlarmId(uint32_t RadioId) {
  if (flarmnet_database) {
    return flarmnet_database->GetFlarmIdItem(RadioId);
  }
  return nullptr;
}

const TCHAR* LookupFLARMDetails(uint32_t RadioId) {
  // try to find flarm from userFile
  auto it = map_flarm_names.find(RadioId);
  if(it != map_flarm_names.end()) {
    return it->second.c_str();
  }

  // try to find flarm from FLARMNet.org File
  if (flarmnet_database) {
    const FlarmId* flarmId = flarmnet_database->GetFlarmIdItem(RadioId);
    if (flarmId != NULL) {
      return flarmId->reg;
    }
  }
  return nullptr;
}

/**
 * @return true if "databases" is modified ( id not exist or exist with other name)
 */
bool AddFlarmLookupItem(uint32_t RadioId, TCHAR *name, bool saveFile) {
    bool added = true;
    auto ib = map_flarm_names.insert(std::make_pair(RadioId, name));
    if (!ib.second) {
      added = (ib.first->second != name);
      // update name if already exist
      if(added) {
        ib.first->second = name;
      }
    }

    if(added && saveFile) {
      SaveFLARMDetails();
    }
    return added;
}
