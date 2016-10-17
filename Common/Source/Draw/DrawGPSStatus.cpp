/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"

//
// This will draw both GPS and LOCK SCREEN status
//
void MapWindow::DrawGPSStatus(LKSurface& Surface, const RECT& rc)
{

  LKSurface::OldFont oldfont = LKSurface::OldFont();
  if ((MapSpaceMode==MSM_WELCOME)||(mode.AnyPan()) ) return; // 100210

  if (!LKLanguageReady) return;

  if (extGPSCONNECT && !(DrawInfo.NAVWarning) && (DrawInfo.SatellitesUsed != 0)) {
	if (LockModeStatus) goto goto_DrawLockModeStatus;
	return;
  }

  static bool firstrun=true;

  if (!extGPSCONNECT) {

    oldfont=Surface.SelectObject(LK8TargetFont);
    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.Color = RGB_WHITE;
    TextInBoxMode.NoSetFont=1;
    TextInBoxMode.AlligneCenter = 1;
    TextInBoxMode.WhiteBorder = 1;
    TextInBoxMode.Border = 1;
    if (ComPortStatus[0]==CPS_OPENKO) {
      TextInBox(Surface, &rc, MsgToken(971), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode); // No ComPort
    } else {
    	if (ComPortStatus[0]==CPS_OPENOK) {
		if ((ComPortRx[0]>0) && !firstrun) {
			// GPS IS MISSING
			TextInBox(Surface, &rc, MsgToken(973), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode);
			firstrun=false; 
		} else {
			// NO DATA RX
			TextInBox(Surface, &rc, MsgToken(972), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode);
		}
	} else  {
		if (ComPortStatus[0]==CPS_EFRAME)  {
			// DATA ERROR
			TextInBox(Surface, &rc, MsgToken(975), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode);
		} else {
			// GPS NOT CONNECTED
			TextInBox(Surface, &rc, MsgToken(974), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode);
		}
	}

    }


  } else
    if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
    oldfont=Surface.SelectObject(LK8TargetFont); // 100210
    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.Color = RGB_WHITE;
    TextInBoxMode.NoSetFont=1;
    TextInBoxMode.AlligneCenter = 1;
    TextInBoxMode.WhiteBorder = 1;
    TextInBoxMode.Border = 1;
    // NO VALID FIX
    TextInBox(Surface, &rc, MsgToken(970), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, &TextInBoxMode);

    }

goto_DrawLockModeStatus:
  if (LockModeStatus) {
    if (oldfont)
      oldfont=Surface.SelectObject(LK8MediumFont);
    else
      Surface.SelectObject(LK8MediumFont);

    TextInBoxMode_t TextInBoxModeL = {0};
    TextInBoxModeL.Color = RGB_WHITE;
    TextInBoxModeL.NoSetFont=1;
    TextInBoxModeL.AlligneCenter = 1;
    TextInBoxModeL.WhiteBorder = 1;
    TextInBoxModeL.Border = 1;
    if (ISPARAGLIDER){
        TextInBox(Surface, &rc,MsgToken(962), (rc.right-rc.left)/2, rc.bottom-((rc.bottom-rc.top)/3), &TextInBoxModeL);
    }
    Surface.SelectObject(LK8MapFont);
    TextInBox(Surface, &rc, MsgToken(1601), (rc.right-rc.left)/2, rc.bottom-((rc.bottom-rc.top)/5), &TextInBoxModeL);
  }

  Surface.SelectObject(oldfont);
  return;
}


