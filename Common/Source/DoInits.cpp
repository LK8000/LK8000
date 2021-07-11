/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#define STATIC_DOINITS
#include "DoInits.h"


bool Valid_DoInit_Position(int position) {
  if (position<0||position>MDI_LAST_DOINIT)
	return false;
  else
	return true;
}


//
// This is a master reset
//
void Reset_All_DoInits(void) {
  #if TESTBENCH
  StartupStore(_T(".... Reset_All_DoInits\n"));
  #endif

  for (int i=MDI_FIRST_DOINIT; i<=MDI_LAST_DOINIT; i++) {
	DoInit[i]=true;
  }

}



void Reset_Single_DoInits(int position) {

  if (!Valid_DoInit_Position(position)) {
	#if TESTBENCH
	StartupStore(_T("... invalid reset single DoInits position=%d\n"),position);
	#endif
	return;
  }

 DoInit[position]=true;

}
