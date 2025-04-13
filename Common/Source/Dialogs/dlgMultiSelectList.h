/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgMultiSelectList.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 13 april 2025
 */

#ifndef DIALOGS_DLMULTISELECTLIST_H
#define DIALOGS_DLMULTISELECTLIST_H

#include <variant>
#include <cstdint>
#include <cstddef>

class CAirspace;

struct im_airspace {
  CAirspace* pAirspace;

  bool operator==(const im_airspace& rhs) const {
    return pAirspace == rhs.pAirspace;
  }
};

struct im_waypoint {
  size_t idx;

  bool operator==(const im_waypoint& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_flarm {
  int idx;

  bool operator==(const im_flarm& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_task_pt {
  int idx;

  bool operator==(const im_task_pt& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_oracle {
  int idx;

  bool operator==(const im_oracle& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_weatherst {
  size_t idx;

  bool operator==(const im_weatherst& rhs) const {
    return idx == rhs.idx;
  }
};

struct im_own_pos {
  bool operator==(const im_own_pos& rhs) const {
    return true;
  }
};

struct im_team {
  bool operator==(const im_team& rhs) const {
    return true;
  }
};

namespace DlgMultiSelect {

using im_element =
  std::variant<im_airspace, im_waypoint, im_flarm, im_task_pt, im_oracle, im_weatherst, im_own_pos, im_team>;

void ShowModal();
void AddItem(im_element&& elmt, double distance);
int GetItemCount();

} // namespace DlgMultiSelect


#endif // DIALOGS_DLMULTISELECTLIST_H
