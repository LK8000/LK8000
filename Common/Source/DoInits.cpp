/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#define STATIC_DOINITS
#include "DoInits.h"



void Init_DoInits(void) {

  for (int i=0; i<=MDI_LAST_DOINIT; i++)
	MasterDoInits[i]=(bool *)NULL;


}

bool Valid_DoInit_Position(int position) {
  if (position<0||position>MDI_LAST_DOINIT)
	return false;
  else
	return true;
}  


//
// ATTENTION ATTENTION ATTENTION
// Of course ONLY USE STATIC LOCATIONS!
//
void Assign_DoInits(bool *location, int position) {

  if (!Valid_DoInit_Position(position)) {
	#if TESTBENCH
	StartupStore(_T("... invalid Assign DoInits position=%d\n"),position);
	#endif
	return;
  }
  MasterDoInits[position]=location;

}


//
// This is a master reset, work in progress
//
void Reset_All_DoInits(void) {
  #if TESTBENCH
  StartupStore(_T(".... Reset_All_DoInits\n"));
  #endif

  for (int i=MDI_FIRST_DOINIT; i<=MDI_LAST_DOINIT; i++) {
	if (MasterDoInits[i]!=(bool *)NULL)
		*MasterDoInits[i]=true;
  }

}



void Reset_Single_DoInits(int position) {
  #if TESTBENCH
  StartupStore(_T(".... Reset_Single_DoInits : %d\n"),position);
  #endif

  if (!Valid_DoInit_Position(position)) {
	#if TESTBENCH
	StartupStore(_T("... invalid reset single DoInits position=%d\n"),position);
	#endif
	return;
  }

  if (MasterDoInits[position]!=(bool *)NULL)
	*MasterDoInits[position]=true;

}


