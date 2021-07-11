/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"

extern POINT startScreen;
extern long VKtime;


//
// This is normally called by DrawMapSpace
//

void MapWindow::LKDrawMultimap_Example(HDC hdc, const RECT rc)
{

  if (DoInit[MDI_MAPASP]) {
	// init statics here and then clear init to false
	DoInit[MDI_MAPASP]=false;
  }

  //
  // X,Y coordinates of last clicked point on screen
  // These coordinates are related to any point clicked, even for a page flip, for bottom bar etc.
  // In some cases, you will read old coordinates because for example after clicking in the center of
  // bottom bar, the page changed out of multimap and entered nearest pages.
  //
  // Beware that they are by no means a trigger: they are always set!
  // If you want to know if user has touched the screen, you need to check for an event!
  //
  int X=startScreen.x;
  int Y=startScreen.y;

  //
  // Duration of key is inside long VKtime, in milliseconds.
  //

  LKWriteBoxedText(hdc, _T("MULTIMAP PAGE EXAMPLE"), 1, 1 , WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);


  TCHAR ttext[100];

  //
  // Attention! Some events are NOT AVAILABLE in "shared" multimaps.
  // Shared multimaps are sharing events and customkeys with the main map.
  // They are hardconfigured in Multimap.cpp
  //

  switch(LKevent) {
	//
	// USABLE EVENTS
	//
	case LKEVENT_NEWRUN:
		// CALLED ON ENTRY: when we select this page coming from another mapspace
		// After the first run on entry, this will be no more passed until next time
		// you enter a new mapspace again.
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
	case LKEVENT_LONGCLICK:
		_stprintf(ttext,_T("Event = LONG CLICK"));
		break;
	case LKEVENT_SHORTCLICK:
		//
		// NOT AVAILABLE IN SHARED MULTIMAPS
		//
		// This is triggered when a click was detected not part of anything else.
		// OR, if you have ActiveMap enabled, anywhere, since UP and DOWN are disabled.
		// Even in this last case, TOPRIGHT, TOPLEFT are managed all the way and SHORTCLICK
		// will not be sent.
		_stprintf(ttext,_T("Event = SHORT CLICK"));
		break;
	case LKEVENT_PAGEUP:
		_tcscpy(ttext,_T("Event = PAGE UP"));
		break;
	case LKEVENT_PAGEDOWN:
		_tcscpy(ttext,_T("Event = PAGE DOWN"));
		break;
	case LKEVENT_TOPLEFT:
		//
		// NOT AVAILABLE IN SHARED MULTIMAPS
		//
		_tcscpy(ttext,_T("Event = TOP LEFT"));
	break;
	case LKEVENT_TOPRIGHT:
		//
		// NOT AVAILABLE IN SHARED MULTIMAPS
		//
		_tcscpy(ttext,_T("Event = TOP RIGHT"));
	break;



	//
	// THESE EVENTS ARE NOT AVAILABLE IN MULTIMAPS!
	//
	case LKEVENT_ENTER:
		// WORKING ONLY in nearest pages
		// click longer on center, like to confirm a selection
		_tcscpy(ttext,_T("Event = ENTER"));
		break;
	case LKEVENT_NEWPAGE:
		// WORKING ONLY in nearest pages
		// swipe gesture up/down produces NEW PAGE in this case
		_tcscpy(ttext,_T("Event = NEW PAGE"));
		break;



	case LKEVENT_NONE:
		// Normally no event.
		_tcscpy(ttext,_T("Event = NONE"));
		break;

	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		_tcscpy(ttext,_T("Event = unknown"));
		break;
  }

  LKWriteBoxedText(hdc, ttext, 1, 50 , WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);

  //
  // Be sure to check that an EVENT was generated, otherwise you are checking even bottombar key presses.
  //
  if (LKevent!=LKEVENT_NONE) {
	_stprintf(ttext,_T("Last coords: X=%d Y=%d  , duration=%ld ms"),X,Y,VKtime);
	LKWriteBoxedText(hdc, ttext, 1, 100 , WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);
  }


  // After using the event, WE MUST CLEAR IT, otherwise it will survive for next run.
  // This can be good for something, though, like automatic redo of last action.
  // You can also clear this event at the end of this function, to know during execution which was
  // the key pressed, but remember to clear it.
  LKevent=LKEVENT_NONE;



}
