/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"
#include "Settings/read.h"
#include "Settings/write.h"

namespace ModelType {

  namespace {

    constexpr list::value_type model_list[] = {
        { GENERIC, _T("GENERIC") },
        { HP31X, _T("HP31x") }, 
        { MEDION_P5, _T("MedionP5") },
        { PNA_MIO, _T("MIO") },
        { NOKIA_500, _T("Nokia500") },
        { PN6000, _T("PN6000") },
        { PNA_NAVIGON, _T("Navigon") },
        { FUNTREK, _T("Holux FunTrek GM-130 / GM-132") },
        { ROYALTEK3200, _T("Medion S3747 / Royaltek BV-3200") },
        { LX_MINI_MAP, _T("LX MiniMap") },
        { BTKA, _T("Keyboard A") },
        { BTKB, _T("KeyBoard B") },
        { BTKC, _T("KeyBoard C") },
        { BTK1, _T("KeyBoard 1") },
        { BTK2, _T("KeyBoard 2") },
        { BTK3, _T("KeyBoard 3") },
        /*
        * list order is important !
        * add new at the end !
        */
    };

    Type_t GlobalModelType = GENERIC;

    constexpr char szRegistryAppInfoBoxModel[] = "AppInfoBoxModel";

  } // namespace


  list::iterator list::begin() {
    return std::begin(model_list);
  }

  list::iterator list::end() {
    return std::end(model_list);
  }

  list::iterator list::find(Type_t id) {
    return std::find_if(list::begin(), list::end(), [&](auto& item) {
        return (std::get<0>(item) == id);
    });
  }

  list::iterator list::find(const tstring_view& name) {
    return std::find_if(list::begin(), list::end(), [&](auto& item) {
        return (std::get<1>(item) == name);
    });
  }

  Type_t get_id(unsigned index) {
    if (index < std::size(model_list)) {
      return std::get<0>(model_list[index]);
    }
    return std::get<0>(model_list[0]);
  }

  unsigned get_index(ModelType::Type_t id) {
    auto it = list::find(id);
    if (it != list::end()) {
      return std::distance(list::begin(), it);
    }
    return 0U;
  }

  const TCHAR* get_name(ModelType::Type_t id) {
    auto it = list::find(id);
    if (it != list::end()) {
      return std::get<1>(*it);
    }
    return std::get<1>(*list::begin());
  }

  void Set(Type_t id) {
    if (list::find(id) != list::end()) {
      GlobalModelType = id;
    } else {
      assert(false); // invalid cast by caller ! ...
    }
  }

  Type_t Get() {
    return GlobalModelType;
  }

  const TCHAR* GetName() {
    return get_name(GlobalModelType);
  }

  bool Set(const TCHAR* name) {
    auto it = list::find(tstring_view(name));
    if (it != list::end()) {
      GlobalModelType = std::get<0>(*it);
      return true;
    }
    return false;
  }

  void ResetSettings() {
    GlobalModelType = GENERIC;
  }

  bool LoadSettings(const char *key, const char *value) {
    return settings::read(key, value, szRegistryAppInfoBoxModel, GlobalModelType);
  }

  void SaveSettings(settings::writer& writer_settings) {
    // We save GlobalModelType, not InfoBoxModel
    writer_settings(szRegistryAppInfoBoxModel, GlobalModelType);
  }

} // namespace  ModelType

#ifdef PNA

//
// A special case for preloading a modeltype directly from
// Default profile, at the very beginning of runtime.
//
bool LoadModelFromProfile()
{
  TCHAR tmpTbuf[MAX_PATH*2];
  char  tmpbuf[MAX_PATH*2];

  LocalPath(tmpTbuf,_T(LKD_CONF), _T(LKPROFILE));

  TestLog(_T("... Searching modeltype inside default profile <%s>"), tmpTbuf);

  FILE *fp = _tfopen(tmpTbuf, _T("rb"));
  if(fp == NULL) {
    StartupStore(_T("... No default profile found%s"),NEWLINE);
    return false;
  }

  AtScopeExit(&) {
    fclose(fp);
  };

  while (fgets(tmpbuf, (MAX_PATH*2)-1, fp) != NULL ) {
    if (strlen(tmpbuf)<21) continue;
    if (strncmp(tmpbuf,"AppInfoBoxModel",15) == 0) { // MUST MATCH!  szRegistryAppInfoBoxModel
      ModelType::Set(static_cast<ModelType::Type_t>(atoi(&tmpbuf[16])));
      TestLog(_T("... ModelType found: <%s> val=%d"), ModelType::GetName(), ModelType::Get());
      return true;
    }
  }
  TestLog(_T("... Modeltype not found in profile, probably Generic PNA is used.\n"));
  return false;
}

#endif
