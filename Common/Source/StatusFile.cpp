/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProfiles.h"


//
// This is useless, because we are not using configurable statusmessages due to lack
// of language support. We should remove all of this, and get rid of StatusMessageData
//
void StatusFileInit() {
  #if TESTBENCH
  StartupStore(TEXT(". StatusFileInit%s"),NEWLINE);
  #endif

  // DEFAULT - 0 is loaded as default, and assumed to exist
  StatusMessageData[0].key = TEXT("DEFAULT");
  StatusMessageData[0].doStatus = true;
  StatusMessageData[0].doSound = true;
  StatusMessageData[0].sound = TEXT("IDR_WAV_DRIP");
  StatusMessageData_Size=1;
  StatusMessageData[0].delay_ms = 1500; // 1.5 s default timing for a status message

}
