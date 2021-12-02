/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"

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
    
  } // namespace


  list::iterator list::begin() {
    return std::begin(model_list);
  }

  list::iterator list::end() {
    return std::end(model_list);
  }

  Type_t get_id(unsigned index) {
    if (index < std::size(model_list)) {
      return std::get<0>(model_list[index]);
    }
    return std::get<0>(model_list[0]);
  }

  unsigned get_index(ModelType::Type_t id) {
    auto it = std::find_if(list::begin(), list::end(), [&](auto& item) {
      return (std::get<0>(item) == id);
    });
    if (it != list::end()) {
      return std::distance(list::begin(), it);
    }
    return 0U;
  }

  const TCHAR* get_name(ModelType::Type_t id) {
    auto it = std::find_if(list::begin(), list::end(), [&](auto& item) {
      return (std::get<0>(item) == id);
    });
    if (it != list::end()) {
      return std::get<1>(*it);
    }
    return std::get<1>(*list::begin());
  }

} // namespace  ModelType

// Parse a MODELTYPE value and set the equivalent model name.
// If the modeltype is invalid or not yet handled, assume that
// the user changed it in the registry or in the profile, and
// correct the error returning false: this will force a Generic Type.

bool SetModelName(ModelType::Type_t id) {
  _tcscpy(GlobalModelName, ModelType::get_name(id));
  return (id != ModelType::GENERIC);
}


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
      GlobalModelType = static_cast<ModelType::Type_t>(atoi(&tmpbuf[16]));
      SetModelName(GlobalModelType);
      TestLog(_T("... ModelType found: <%s> val=%d"), GlobalModelName, GlobalModelType);
      return true;
    }
  }
  TestLog(_T("... Modeltype not found in profile, probably Generic PNA is used.\n"));
  return false;
}

bool SetModelType() {
	GlobalModelType = ModelType::GENERIC;
  return SetModelName(ModelType::GENERIC);
}

#endif