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


void MapWindow::LKDrawFlarmRadar(HDC hdc, const RECT rc)
{

  if (DoInit[MDI_FLARMRADAR]) {
	// init statics here

	DoInit[MDI_FLARMRADAR]=false;
  }


  // No need to paint 4.4 on the topleft corner now, but we shall do it eventually later.
  // So it is better not to use the top left corner for printing things.
  LKWriteBoxedText(hdc, _T("4.4 FLARM RADAR"), 1, 1 , 0, WTALIGN_LEFT);


  TCHAR ttext[100];
  
  switch(LKevent) {
	//
	// USABLE EVENTS
	// 
	case LKEVENT_NEWRUN:
		// CALLED ON ENTRY: when we select this page coming from another mapspace
		_tcscpy(ttext,_T("Event = NEW RUN"));
		break;
	case LKEVENT_UP:
		// click on upper part of screen, excluding center
		_tcscpy(ttext,_T("Event = UP"));
		break;
	case LKEVENT_DOWN:
		// click on lower part of screen,  excluding center
		_tcscpy(ttext,_T("Event = DOWN"));
		break;
	case LKEVENT_ENTER:
		// click longer on center, like to confirm a selection
		_tcscpy(ttext,_T("Event = ENTER"));
		break;
	case LKEVENT_NEWPAGE:
		// swipe gesture up/down produces NEW PAGE in this case
		_tcscpy(ttext,_T("Event = NEW PAGE"));
		break;

	
	case LKEVENT_PAGEUP:
		// NOT AVAILABLE
		_tcscpy(ttext,_T("Event = PAGE UP"));
		break;
	case LKEVENT_PAGEDOWN:
		// NOT AVAILABLE
		_tcscpy(ttext,_T("Event = PAGE DOWN"));
		break;
	case LKEVENT_NONE:
		// Normally no event.
		_tcscpy(ttext,_T("Event = NONE"));
		break;
	default:
		// THIS SHOULD NEVER HAPPEN, but CHECK FOR IT
		_tcscpy(ttext,_T("Event = unknown"));
		break;
  }

  LKWriteBoxedText(hdc, ttext, 1, 50 , 0, WTALIGN_LEFT);

  // After using the event, WE MUST CLEAR IT, otherwise it will survive for next run.
  // This can be good for something, though, like automatic redo of last action.
  // You can also clear this event at the end of this function, to know during execution which was
  // the key pressed, but remember to clear it.
  LKevent=LKEVENT_NONE;



}

