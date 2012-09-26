/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"

extern int XstartScreen, YstartScreen;
extern long VKtime;



void MapWindow::LKDrawMultimap_Radar(HDC hdc, const RECT rc)
{

  if (DoInit[MDI_MAPRADAR]) {
	// init statics here and then clear init to false
	DoInit[MDI_MAPRADAR]=false;
  }


  	RECT frc = rc;

    frc.bottom = frc.bottom - BottomSize - NIBLSCALE(2);
  	LKDrawFlarmRadar(hdc,frc);




}

