/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "LKInterface.h"
#if defined(PNA) && defined(UNDER_CE)
#include "Devices/LKHolux.h"
#include "Devices/LKRoyaltek3200.h"
#endif
#include "DoInits.h"
#include "Multimap.h"
#include "Sound/Sound.h"

extern int ProcessSubScreenVirtualKey(int X, int Y, long keytime, short vkmode);
long VKtime=0;

// vkmode 0=normal 1=gesture up 2=gesture down
// however we consider a down as up, and viceversa
int ProcessVirtualKey(int X, int Y, long keytime, short vkmode) {

#define VKTIMELONG 1500
	short yup, ydown;
	short i, j;
	short numpages=0;

	static short s_xright=0, s_xleft=0;

	short shortpress_yup, shortpress_ydown;
	short longpress_yup, longpress_ydown;

	static short s_bottomY=0;
	#if 0 // 121123 CHECK AND REMOVE
	static short oldMapSpaceMode=0;
	#endif

	bool dontdrawthemap=(DONTDRAWTHEMAP);
	VKtime=keytime;

	#ifdef DEBUG_PROCVK
	TCHAR buf[100];
	_stprintf(buf,_T("R=%d,%d,%d,%d, X=%d Y=%d kt=%ld"),0, 0,
	ScreenSizeX, ScreenSizeY,X,Y,keytime);
	DoStatusMessage(buf);
	#endif


	if (DoInit[MDI_PROCESSVIRTUALKEY]) {

		// calculate left and right starting from center
		s_xleft=(MapWindow::MapRect.right+MapWindow::MapRect.left)/2 -(MapWindow::MapRect.right-MapWindow::MapRect.left)/6;
		s_xright=(MapWindow::MapRect.right+MapWindow::MapRect.left)/2 + (MapWindow::MapRect.right-MapWindow::MapRect.left)/6;

		// same for bottom navboxes: they do not exist in infobox mode
		s_bottomY=MapWindow::Y_BottomBar-NIBLSCALE(2);
#if TESTBENCH
                StartupStore(_T("... Virtualkeys: s_xleft=%d s_xright=%d s_bottomY=%d%s"),s_xleft,s_xright,s_bottomY, NEWLINE);
#endif

		DoInit[MDI_PROCESSVIRTUALKEY]=false;
	}


        // LK v6: check we are not out of MapRect bounds.
        if (X<MapWindow::MapRect.left||X>MapWindow::MapRect.right||Y<MapWindow::MapRect.top||Y>MapWindow::MapRect.bottom)
            return ProcessSubScreenVirtualKey(X,Y,keytime,vkmode);

        if (MapSpaceMode==MSM_WELCOME) {
            SetModeType(LKMODE_MAP, MP_MOVING);
            LKevent=LKEVENT_NONE;
            NextModeIndex();
            PreviousModeIndex();
            MapWindow::RefreshMap();
            LKSound(_T("LK_BEEP0.WAV"));
            return 0;
        }


	// 120602 fix
	// TopSize is dynamically assigned by DrawNearest,Drawcommon, DrawXX etc. so we cannot make static yups
        // v5 bug fix: TopSize is available only after DrawNearest is executed. So it is now reset at 0
        // on ChangeScreen, but it is a mistake. Really we dont need TopSize, it was used for Long center key
        // press which is no more in use.
	//
	longpress_yup=(short)((MapWindow::Y_BottomBar-TopSize)/3.7)+TopSize;
	longpress_ydown=(short)(MapWindow::Y_BottomBar-(MapWindow::Y_BottomBar/3.7));
	shortpress_yup=(short)((MapWindow::Y_BottomBar-TopSize)/2.7)+TopSize;
	shortpress_ydown=(short)(MapWindow::Y_BottomBar-(MapWindow::Y_BottomBar/2.7));

	// do not consider navboxes, they are processed separately
	// These are coordinates for up down center VKs
	// yup and ydown are used normally on nearest page item selection, but also for real VK
	// that currently are almost unused.

	if (DrawBottom) {
		// Native LK mode: always fullscreen mode
		// If long click, we are processing an Enter, and we want a wider valid center area
		if ( keytime>=(VKSHORTCLICK*2)) {
			yup=longpress_yup;
			ydown=longpress_ydown;
		} else {
			yup=shortpress_yup;
			ydown=shortpress_ydown;
		}
	} else {
		// This could happen only in Ibox mode. We should never fall here.
		yup=(short)(ScreenSizeY/2.7);
		ydown=(short)(ScreenSizeY-(ScreenSizeY/2.7));
		#if TESTBENCH
		StartupStore(_T("...... DrawBottom FALSE in virtual key processing!\n"));
		#endif
	}

	#ifdef DEBUG_PROCVK
	TCHAR buf[100];
	#endif

	// Handle fullscreen 8000 mode
	// sound clicks require some attention here
	if (DrawBottom && !MapWindow::mode.AnyPan() && vkmode==LKGESTURE_NONE) {
		//
		// CLICKS on NAVBOXES, any MapSpaceMode ok
		//
		if (Y>= s_bottomY ) { // TESTFIX 090930

			if ( X>s_xright ) {
				// standard configurable mode
				if (keytime >=CustomKeyTime) {
					// 2 is right key
					if (ScreenKeyHandler(ScreenKey::CKI_BOTTOMRIGHT)) return 0;
				}
				#ifdef DEBUG_PROCVK
				_stprintf(buf,_T("RIGHT in limit=%d"),MapWindow::Y_BottomBar-NIBLSCALE(20));
				DoStatusMessage(buf);
				#endif
				BottomBarChange(true); // advance
				BottomSounds();
				MapWindow::RefreshMap();
				return 0;
			}
			if ( X<s_xleft ) { // following is ugly
				if (keytime >=CustomKeyTime) {
					// 1 is left key
					if (ScreenKeyHandler(ScreenKey::CKI_BOTTOMLEFT)) return 0;
				}

				#ifdef DEBUG_PROCVK
				_stprintf(buf,_T("LEFT in limit=%d"),MapWindow::Y_BottomBar-NIBLSCALE(20));
				DoStatusMessage(buf);
				#endif
				BottomBarChange(false); // backwards
				BottomSounds();
				MapWindow::RefreshMap();
				return 0;
			}
			#ifdef DEBUG_PROCVK
			_stprintf(buf,_T("CENTER in limit=%d"),MapWindow::Y_BottomBar-NIBLSCALE(20));
			DoStatusMessage(buf);
			#endif

			//
			// VIRTUAL CENTER KEY HANDLING
			//
			// long press on center navbox
			// Activate following choices for testing and experimenting. Always disable for real usage.
#if (0)
			// Output NMEA to device
			if (keytime>1000) {
				PlayResource(TEXT("IDR_WAV_HIGHCLICK"));

				devWriteNMEAString(devA(),_T("$PGRMCE"));
				Message::AddMessage(1000, 3, _T("NMEA out $PGRMCE"));
				return 0;
			}
#endif
#if (0)
			// Simulate incoming NMEA string
			if (keytime>1000) {
				static TCHAR mbuf[200];
				_stprintf(mbuf,_T("$VARIO,1010.18,0.0,0.00,2.34,2,000.0,000.0*51\n"));
				NMEAParser::ParseNMEAString(0, (TCHAR *) mbuf, &GPS_INFO);
				return 0;
			}
#endif
#if (0)	// TESTKEY
			// Print a message on the screen for debugging purposes
			TCHAR mbuf[100];
			if (keytime>1000) {
				// _stprintf(mbuf,_T("Cache MCA %d/%d F=%d"), Cache_Hits_MCA, Cache_Calls_MCA, Cache_False_MCA );
				char *point;
				point=(char*)&mbuf;
				*point++='A';
				*point++='\0';
				*point++='B';
				*point++='\0';
				*point++=0x06;
				*point++=0x01;
				*point++='C';
				*point++='\0';
				*point++='\0';
				*point++='\0';
				//mbuf[1]=0xc4;
				//mbuf[1]=0x86;
				Message::AddMessage(20000, 3, mbuf);
				return 0;
			}
#endif

#if (0)
			if (keytime>=CustomKeyTime) {
				if (OvertargetMode==OVT_MAXMODE) OvertargetMode=0;
				else OvertargetMode++;
				PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
				return 0;
			}
#endif

#if (0)
			if (keytime>=CustomKeyTime) {
				PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
				extern bool RunSignature(void);
				RunSignature();

				return 0;
			}
#endif

#if (0)
			// Long press in center screen bottom bar
			if (keytime>=CustomKeyTime) {
				PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
				ReinitScreen();
				return 0;
			}
#endif

			// REAL USAGE, ALWAYS ACTIVATE
			#if (1)
			// standard configurable mode
			if (keytime >=CustomKeyTime) {
				// 0 is center key
				if (ScreenKeyHandler(ScreenKey::CKI_BOTTOMCENTER)) return 0;
			}
			#endif

			// normally, we fall down here.
			// If CustomKeyHandler returned false, back as well here (nothing configured in custom).
			//
			///// If we are clicking on center bottom bar while still in welcome page, set map before nextmode.
			NextModeIndex();
			MapWindow::RefreshMap();
			SoundModeIndex();

			return 0;
		// End click on navboxes
		} else
		// CLICK ON SORTBOX line at the top, only with no map and only for enabled pages
		if ( (Y<=SortBoxY[MapSpaceMode]) &&
			( MapSpaceMode == MSM_LANDABLE || MapSpaceMode==MSM_AIRPORTS ||
			MapSpaceMode==MSM_NEARTPS || MapSpaceMode==MSM_TRAFFIC ||
			MapSpaceMode==MSM_AIRSPACES || MapSpaceMode==MSM_THERMALS ||
			MapSpaceMode==MSM_COMMON || MapSpaceMode==MSM_RECENT) ) {

			// only search for 1-3, otherwise it's the fourth (fifth really)
			// we don't use 0 now
			for (i=0, j=4; i<4; i++) { // i=1 original 090925 FIX
				if (X <SortBoxX[MapSpaceMode][i]) {
					j=i;
					break;
				}
			}

			#if 0 // 121123 CHECK AND REMOVE
			// 120504 if we are clicking on the already selected sort button, within the same mapspacemode,
			// then simulate a gesture down to advance to next page, if available.
			if ( (MapSpaceMode==oldMapSpaceMode && SortedMode[MapSpaceMode]==j)  ||
			     (MapSpaceMode==MSM_COMMON) || (MapSpaceMode==MSM_RECENT) ) {
				vkmode=LKGESTURE_DOWN;
				goto shortcut_gesture;
			} else {
				oldMapSpaceMode=MapSpaceMode; // becomes current
			}
			#else
			if ( (SortedMode[MapSpaceMode]==j)  ||
			     (MapSpaceMode==MSM_COMMON) || (MapSpaceMode==MSM_RECENT) ) {
				vkmode=LKGESTURE_DOWN;
				goto shortcut_gesture;
			}
			#endif

			switch(MapSpaceMode) {
				case MSM_LANDABLE:
				case MSM_AIRPORTS:
				case MSM_NEARTPS:
							SortedMode[MapSpaceMode]=j;
							LKForceDoNearest=true;
							PlayResource(TEXT("IDR_WAV_CLICK"));
							break;
				case MSM_TRAFFIC:
							SortedMode[MapSpaceMode]=j;
							// force immediate resorting
							LastDoTraffic=0;
							PlayResource(TEXT("IDR_WAV_CLICK"));
							break;
				case MSM_AIRSPACES:
							SortedMode[MapSpaceMode]=j;
							PlayResource(TEXT("IDR_WAV_CLICK"));
							break;

				case MSM_THERMALS:
							SortedMode[MapSpaceMode]=j;
							// force immediate resorting
							LastDoThermalH=0;
							PlayResource(TEXT("IDR_WAV_CLICK"));
							break;
				default:
							DoStatusMessage(_T("ERR-022 UNKNOWN MSM in VK"));
							break;
			}
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			MapWindow::RefreshMap();

			return 0;
		// end sortbox
		}
	// end newmap  with no gestures
	}

	// REAL virtual keys
	// Emulate real keypresses with wParam

shortcut_gesture:
	// UP gesture
	if (vkmode>LKGESTURE_NONE) {
		// WE MANAGE GESTURES IN ALL MAPSPACES
		switch(MapSpaceMode) {
			case MSM_LANDABLE:
			case MSM_AIRPORTS:
			case MSM_NEARTPS:
						LKForceDoNearest=true;
						numpages=Numpages;
						break;
			case MSM_COMMON:
						LKForceDoCommon=true;
						numpages=Numpages;
						break;
			case MSM_RECENT:
						LKForceDoRecent=true;
						numpages=Numpages;
						break;
			case MSM_AIRSPACES:
						numpages=Numpages;
						break;
			case MSM_TRAFFIC:
						numpages=Numpages;
						break;
			case MSM_THERMALS:
						numpages=Numpages;
						break;
			default:
						break;
		}
		SelectedRaw[MapSpaceMode]=0;

		switch(vkmode) {
			// SCROLL DOWN
			case LKGESTURE_DOWN:
				// no pagedown for main map.. where do you want to go??
				if (NOTANYPAN && IsMultiMapNoMain()) {
					LKevent=LKEVENT_PAGEDOWN;
					MapWindow::RefreshMap();
					return 0;
				}
				// careful, selectedpage starts from 0
				if (++SelectedPage[MapSpaceMode] >=numpages) {
					PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
					SelectedPage[MapSpaceMode]=0;
				} else {
					PlayResource(TEXT("IDR_WAV_CLICK"));
				}
				LKevent=LKEVENT_NEWPAGE;
				MapWindow::RefreshMap();
				return 0;
			// SCROLL UP
			case LKGESTURE_UP:
				// no pagedown for main map.. where do you want to go??
				if (NOTANYPAN && IsMultiMapNoMain()) {
					LKevent=LKEVENT_PAGEUP;
					MapWindow::RefreshMap();
					return 0;
				}
				if (--SelectedPage[MapSpaceMode] <0) {
					PlayResource(TEXT("IDR_WAV_CLICK"));
					SelectedPage[MapSpaceMode]=(numpages-1);
				} else {
					if (SelectedPage[MapSpaceMode]==0) {
						PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
					} else {
						PlayResource(TEXT("IDR_WAV_CLICK"));
					}
				}
				LKevent=LKEVENT_NEWPAGE;
				MapWindow::RefreshMap();
				return 0;
			case LKGESTURE_RIGHT:
gesture_right:
				NextModeType();
				MapWindow::RefreshMap();

                    // Notice: MultiMap has its own sounds. We come here when switching pages, but with
					// an exception: from moving map we generate currently a direct NextModeType from
					// MapWndProc, and thus we dont get ProcessVirtualKeys for that single case.
					// We should not be playing a CLICK sound while we are playing the MM tone, or
					// it wont come up !
					if (ModeIndex!=LKMODE_MAP) {
						if (CURTYPE == 0)
							PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
						else
							PlayResource(TEXT("IDR_WAV_CLICK"));
					}
				return 0;

				break;

			case LKGESTURE_LEFT:
gesture_left:
				PreviousModeType();
				MapWindow::RefreshMap();
                if (ModeIndex!=LKMODE_MAP) {
                    if (CURTYPE == 0)
                        PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
                    else
                        PlayResource(TEXT("IDR_WAV_CLICK"));
                }
				return 0;

				break;
			default:
				return 0;
		}

		return 0;
	}

	if (!MapWindow::mode.AnyPan() && (IsMultiMap()||MapSpaceMode==MSM_MAPTRK)) {
		if (keytime>=(VKSHORTCLICK*4)) {
			LKevent=LKEVENT_LONGCLICK;
			MapWindow::RefreshMap();
			return 0;
		}
	}

	// UNGESTURES:
	// No need to use gestures if clicking on right or left center border screen
	// This will dramatically speed up the user interface in turbulence
	if (dontdrawthemap) {
		if (Y>longpress_yup && Y<longpress_ydown) {
			if (UseUngestures || !ISPARAGLIDER) {
				if (X<=MapWindow::X_Left)  goto gesture_left;
				if (X>=MapWindow::X_Right) goto gesture_right;
			}
		}
	}

	///
	/// REMOVE. ActiveMap always false
	///
	///if (!MapWindow::mode.AnyPan() && IsMultiMapNoMain() && ActiveMap) {
	///	LKevent=LKEVENT_SHORTCLICK;
	///	MapWindow::RefreshMap();
	///	return 0;
	///}

	if (Y<yup) {
		// we are processing up/down in mapspacemode i.e. browsing waypoints on the page
		if (dontdrawthemap) {
			if (MapSpaceMode<=MSM_MAP) {
				// DoStatusMessage(_T("DBG-032-A event up not used here"));
				return 0;
			}
		PlayResource(TEXT("IDR_WAV_CLICK"));
			LKevent=LKEVENT_UP;
			MapWindow::RefreshMap();
			// DoStatusMessage(_T("DBG-032-B event up used here"));
			return 0;
		}
        PlayResource(TEXT("IDR_WAV_CLICK"));
		if (keytime>=VKTIMELONG)
			return 0xc1;
		else
			return 38;
	}
	if (Y>ydown) {
		if (dontdrawthemap) {
			if (MapSpaceMode<=MSM_MAP) return 0;
		PlayResource(TEXT("IDR_WAV_CLICK"));
			LKevent=LKEVENT_DOWN;
			MapWindow::RefreshMap();
			return 0;
		}
        PlayResource(TEXT("IDR_WAV_CLICK"));
		if (keytime>=VKTIMELONG)
			return 0xc2;
		else
			return 40;
	}

	// This will not be detected in case of UP and DOWN was detected, of course.
	// We must handle this separately, before checking for UP DOWN, above.
	if (!MapWindow::mode.AnyPan() && IsMultiMap()) {
		LKevent=LKEVENT_SHORTCLICK;
		MapWindow::RefreshMap();
		return 0;
	}

	// no click for already clicked events


		// If in mapspacemode process ENTER
		if ( (keytime>=(VKSHORTCLICK*2)) && dontdrawthemap && !IsMultiMap()) {
			LKSound(_T("LK_BEEP1.WAV"));
			LKevent=LKEVENT_ENTER;
			MapWindow::RefreshMap();
			return 0;
		}
/*
		// do not process enter in panmode, unused
		if ( !MapWindow::mode.AnyPan() ) {
	             DoStatusMessage(_T("Virtual ENTER"));
		     return 13;
		}
*/

		//
		// Here we are when short clicking in the center area, not an up and not a down.. a center.
		// We do nothing.
		//


		if (SIMMODE) {
			if ( MapWindow::mode.AnyPan() && ISPARAGLIDER) return 99; // 091221 return impossible value
			else return 0;
		} else {
			return 0;
		}
	DoStatusMessage(_T("VirtualKey Error"));
	return 0;
}



//
// LK v6 Keyclicks out of MapRect but inside DrawRect. What we call SubScreen area.
//
int ProcessSubScreenVirtualKey(int X, int Y, long keytime, short vkmode) {

    #if TESTBENCH
    TCHAR buf[100];
    _stprintf(buf,_T("SubScreen Key: X=%d Y=%d kt=%ld vk=%d"),X,Y,keytime,vkmode);
    DoStatusMessage(buf);
    StartupStore(_T(".... %s%s"),buf,NEWLINE);
    #endif

    return 0; // unmanaged keypress

}
