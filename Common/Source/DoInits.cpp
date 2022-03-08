/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include <array>
#include "MessageLog.h"
#include "DoInits.h"

namespace {

template <std::size_t ... Is>
std::array<bool, sizeof...(Is)>
Init_DoInit(std::index_sequence<Is...>) {
  return {{(void(Is), true)...}};
}

DoInit_t Init_DoInit() {
  return Init_DoInit(std::make_index_sequence<MDI_LAST_DOINIT>());
}

} // namespace

DoInit_t DoInit = Init_DoInit();

void Reset_Single_DoInits(MDI_t position) {
  if (position < DoInit.size()) {
    DoInit[position] = true;
  }
  else {
    TestLog(_T("... invalid reset single DoInits position=%u"), position);
  }
}
