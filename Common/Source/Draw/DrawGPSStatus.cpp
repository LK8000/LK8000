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
void MapWindow::DrawGPSStatus(HDC hDC, const RECT rc)
{

  HFONT oldfont=NULL;
  if ((MapSpaceMode==MSM_WELCOME)||(mode.AnyPan()) ) return; // 100210

  if (!LKLanguageReady) return;

  if (extGPSCONNECT && !(DrawInfo.NAVWarning) && (DrawInfo.SatellitesUsed != 0)) {
	if (LockModeStatus) goto goto_DrawLockModeStatus;
	return;
  }

  static bool firstrun=true;

  if (!extGPSCONNECT) {

    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont); 
    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.Color = RGB_WHITE;
    TextInBoxMode.NoSetFont=1;
    TextInBoxMode.AlligneCenter = 1;
    TextInBoxMode.WhiteBorder = 1;
    TextInBoxMode.Border = 1;
    if (ComPortStatus[0]==CPS_OPENKO) {
      TextInBox(hDC, gettext(_T("_@M971_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, &TextInBoxMode); // No ComPort
    } else {
    	if (ComPortStatus[0]==CPS_OPENOK) {
		if ((ComPortRx[0]>0) && !firstrun) {
			// Gps is missing
      TextInBox(hDC, gettext(_T("_@M973_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, &TextInBoxMode);
			firstrun=false; // 100214
		} else {
			// No Data Rx
      TextInBox(hDC, gettext(_T("_@M972_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, &TextInBoxMode);
		}
	} else  {
		if (ComPortStatus[0]==CPS_EFRAME)  {
			// Data error
      TextInBox(hDC, gettext(_T("_@M975_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, &TextInBoxMode);
		} else {
			// Not Connected
      TextInBox(hDC, gettext(_T("_@M974_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, &TextInBoxMode);
		}
	}

    }


  } else
    if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont); // 100210
    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.Color = RGB_WHITE;
    TextInBoxMode.NoSetFont=1;
    TextInBoxMode.AlligneCenter = 1;
    TextInBoxMode.WhiteBorder = 1;
    TextInBoxMode.Border = 1;
    // No Valid Fix
    TextInBox(hDC, gettext(_T("_@M970_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, &TextInBoxMode);

    }

goto_DrawLockModeStatus:
  if (LockModeStatus) {
    if (oldfont!=NULL)
      oldfont=(HFONT)SelectObject(hDC,LK8MediumFont);
    else
      SelectObject(hDC,LK8MediumFont);

    TextInBoxMode_t TextInBoxModeL = {0};
    TextInBoxModeL.Color = RGB_WHITE;
    TextInBoxModeL.NoSetFont=1;
    TextInBoxModeL.AlligneCenter = 1;
    TextInBoxModeL.WhiteBorder = 1;
    TextInBoxModeL.Border = 1;
    if (ISPARAGLIDER) TextInBox(hDC, gettext(_T("_@M962_")), (rc.right-rc.left)/2, rc.bottom-((rc.bottom-rc.top)/3), 0, &TextInBoxModeL);
    SelectObject(hDC,LK8MapFont);
    TextInBox(hDC, gettext(_T("_@M1601_")), (rc.right-rc.left)/2, rc.bottom-((rc.bottom-rc.top)/5), 0, &TextInBoxModeL);
  }

  SelectObject(hDC,oldfont);
  return;
}


