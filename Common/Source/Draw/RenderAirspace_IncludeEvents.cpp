/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

 //
 // THIS PART IS COMMON TO RenderAirspace and RenderNearAirspace, for the time being.
 // The code must be cleaned up further more! This is hack.
 // THIS CODE IS INCLUDED IN BOTH FILES, AS IF IT WAS THERE, and THUS IT IS DUPLICATED IN THE BINARY!
 //

	  switch(LKevent) {
		case LKEVENT_NEWRUN:
			// CALLED ON ENTRY: when we select this page coming from another mapspace
			fZOOMScale = 1.0;
			fHeigtScaleFact = 1.0;
		break;
		case LKEVENT_UP:
			// click on upper part of screen, excluding center
			if(bHeightScale)
			  fHeigtScaleFact /= ZOOMFACTOR;
			else
			  fZOOMScale /= ZOOMFACTOR;
			break;

		case LKEVENT_DOWN:
			// click on lower part of screen,  excluding center
			if(bHeightScale)
			  fHeigtScaleFact *= ZOOMFACTOR;
			else
		  	  fZOOMScale *= ZOOMFACTOR;
			break;

		case LKEVENT_LONGCLICK:
		     for (k=0 ; k <= Sideview_iNoHandeldSpaces; k++)
			 {
			   if( Sideview_pHandeled[k].psAS != NULL)
			   {
				 if (PtInRect(XstartScreen,YstartScreen,Sideview_pHandeled[k].rc ))
				 {
				   if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_BTONE4"));
				   dlgAirspaceDetails(Sideview_pHandeled[k].psAS);       // dlgA
				   bFound = true;
				 }
			   }
			 }
			 if (PtInRect(XstartScreen, YstartScreen,rc ))
			   bHeightScale = true;
			 if (PtInRect(XstartScreen, YstartScreen,rct ))
			   bHeightScale = false;
	     break;

		case LKEVENT_PAGEUP:
#ifdef OFFSET_SETP
			if(bHeightScale)
			  fOffset -= OFFSET_SETP;
			else
#endif
			{
			  if(*iSplit == SIZE1) *iSplit = SIZE0;
			  if(*iSplit == SIZE2) *iSplit = SIZE1;
			  if(*iSplit == SIZE3) *iSplit = SIZE2;
			}
		break;
		case LKEVENT_PAGEDOWN:
#ifdef OFFSET_SETP
			if(bHeightScale)
			  fOffset += OFFSET_SETP;
			else
#endif
			{
			  if(*iSplit == SIZE2) *iSplit = SIZE3;
			  if(*iSplit == SIZE1) *iSplit = SIZE2;
			  if(*iSplit == SIZE0) *iSplit = SIZE1;
			}
		break;

	  }
	  //LKevent=LKEVENT_NONE;

	  // Current_Multimap_SizeY is global, and must be used by all multimaps!
	  // It is defined in Multimap.cpp and as an external in Multimap.h
	  // It is important that it is updated, because we should resize terrain
	  // only if needed! Resizing terrain takes some cpu and some time.
	  // So we need to know when this is not necessary, having the same size of previous
	  // multimap, if we are switching.
	  // The current implementation is terribly wrong by managing resizing of sideview in
	  // each multimap: it should be done by a common layer.
	  // CAREFUL:
	  // If for any reason DrawTerrain() is called after resizing to 0 (full sideview)
	  // LK WILL CRASH with no hope to recover.
	  if(Current_Multimap_SizeY != *iSplit)
	  {
		Current_Multimap_SizeY=*iSplit;
		SetSplitScreenSize(*iSplit);
		rc.top     = (long)((double)(rci.bottom-rci.top  )*fSplitFact);
		rct.bottom = rc.top ;
	  }


