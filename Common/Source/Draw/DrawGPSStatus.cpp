/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"

//
// This will draw both GPS and LOCK SCREEN status
//
void MapWindow::DrawGPSStatus(LKSurface& Surface, const RECT& rc) {
  const TCHAR * message = nullptr;

  if ((MapSpaceMode == MSM_WELCOME) || (mode.AnyPan())) {
    return;  // 100210
  }
  if (extGPSCONNECT && !(DrawInfo.NAVWarning) && (DrawInfo.SatellitesUsed != 0)) {
    if (LockModeStatus) {
      goto goto_DrawLockModeStatus;
    }
    return;
  }

  if (!extGPSCONNECT) {
    message = MsgToken<974>(); // GPS NOT CONNECTED
    for (const auto& dev : DeviceList) {
      ScopeLock Lock(CritSec_Comm);
      if (dev.IsReady()) {
        message = nullptr;
      }
    }
  } else if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
    message = MsgToken<970>(); // NO VALID FIX
  }

  if (message) {
    auto oldfont = Surface.SelectObject(LK8TargetFont);

    TextInBoxMode_t TextInBoxMode = {};
    TextInBoxMode.Color = RGB_WHITE;
    TextInBoxMode.NoSetFont = 1;
    TextInBoxMode.AlligneCenter = 1;
    TextInBoxMode.WhiteBorder = 1;
    TextInBoxMode.Border = 1;
    TextInBox(Surface, &rc, message, (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 3, &TextInBoxMode);

    Surface.SelectObject(oldfont);
  }

goto_DrawLockModeStatus:
  if (LockModeStatus) {
    auto oldfont = Surface.SelectObject(LK8MediumFont);

    TextInBoxMode_t TextInBoxModeL = {};
    TextInBoxModeL.Color = RGB_WHITE;
    TextInBoxModeL.NoSetFont = 1;
    TextInBoxModeL.AlligneCenter = 1;
    TextInBoxModeL.WhiteBorder = 1;
    TextInBoxModeL.Border = 1;
    if (ISPARAGLIDER) {
      TextInBox(Surface, &rc, MsgToken<962>(), (rc.right - rc.left) / 2, rc.bottom - ((rc.bottom - rc.top) / 3),
                &TextInBoxModeL);
    }
    Surface.SelectObject(LK8MapFont);
    TextInBox(Surface, &rc, MsgToken<1601>(), (rc.right - rc.left) / 2, rc.bottom - ((rc.bottom - rc.top) / 5),
              &TextInBoxModeL);

    Surface.SelectObject(oldfont);
  }
}
