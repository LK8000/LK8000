/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "RGB.h"



void MapWindow::DrawMapScale(HDC hDC, const RECT rc /* the Map Rect*/, 
                             const bool ScaleChangeFeedback)
{
  static short terrainwarning=0;

    TCHAR Scale[80];
    TCHAR Scale2[80];
    TCHAR TEMP[20];
    POINT Start, End;
    COLORREF origcolor = SetTextColor(hDC, OverColorRef);

    HPEN hpOld;
    hpOld = (HPEN)SelectObject(hDC, hpMapScale2);


    // TODO use Appearance font Hight to calculate correct offset

    Start.x = rc.right-NIBLSCALE(6); End.x = rc.right-NIBLSCALE(6);

    Start.y = rc.bottom-BottomSize-NIBLSCALE(4); // 100922
    End.y = Start.y - NIBLSCALE(42);
    DrawSolidLine(hDC,Start,End, rc);

    Start.x = rc.right-NIBLSCALE(11); End.x = rc.right-NIBLSCALE(6);
    End.y = Start.y;
    DrawSolidLine(hDC,Start,End, rc);

     Start.y = Start.y - NIBLSCALE(42); End.y = Start.y;
    DrawSolidLine(hDC,Start,End, rc);

    SelectObject(hDC, hpOld);

    _tcscpy(Scale2,TEXT(""));

    // warn about missing terrain
    if (!CALCULATED_INFO.TerrainValid) {
	if (terrainwarning < 120) {
		// LKTOKEN _@M1335_ " TERRAIN?"
		_tcscat(Scale2, gettext(TEXT("_@M1335_")));
		terrainwarning++;
	} else  {
		// LKTOKEN _@M1336_ " T?"
		_tcscat(Scale2, gettext(TEXT("_@M1336_")));
		terrainwarning=120;
	}
    } else terrainwarning=0;

    if (ActiveMap) {
      _tcscat(Scale2, gettext(TEXT("_@M1661_"))); // ACT
    }
    if (UseTotalEnergy) {
      _tcscat(Scale2, TEXT("[TE]")); // Total Energy indicator
    }
    if (zoom.AutoZoom()) {
		// LKTOKEN _@M1337_ " AZM"
      _tcscat(Scale2, gettext(TEXT("_@M1337_")));
    }
    if (mode.AnyPan()) {
		// LKTOKEN _@M1338_ " PAN"
      _tcscat(Scale2, gettext(TEXT("_@M1338_")));
    }

    if (DrawBottom) {
	switch(BottomMode) {
		case BM_TRM:
				// LKTOKEN _@M1340_ " TRM0"
      			_tcscat(Scale2, gettext(TEXT("_@M1340_")));
			break;
		case BM_CRU:
				// LKTOKEN _@M1341_ " NAV1"
      			_tcscat(Scale2, gettext(TEXT("_@M1341_")));
			break;
		case BM_HGH:
				// LKTOKEN _@M1342_ " ALT2"
      			_tcscat(Scale2, gettext(TEXT("_@M1342_")));
			break;
		case BM_AUX:
				// LKTOKEN _@M1343_ " STA3"
      			_tcscat(Scale2, gettext(TEXT("_@M1343_")));
			break;
		case BM_TSK:
				// LKTOKEN _@M1344_ " TSK4"
      			_tcscat(Scale2, gettext(TEXT("_@M1344_")));
			break;
		case BM_ALT:
				// LKTOKEN _@M1345_ " ATN5"
      			_tcscat(Scale2, gettext(TEXT("_@M1345_")));
			break;
		case BM_SYS:
				// LKTOKEN _@M1346_ " SYS6"
      			_tcscat(Scale2, gettext(TEXT("_@M1346_")));
			break;
		case BM_CUS2:
				// LKTOKEN _@M1347_ " CRU7"
      			_tcscat(Scale2, gettext(TEXT("_@M1347_")));
			break;
		case BM_CUS3:
				// LKTOKEN _@M1348_ " FIN8"
      			_tcscat(Scale2, gettext(TEXT("_@M1348_")));
			break;
		case BM_CUS:
				// LKTOKEN _@M1349_ " AUX9"
      			_tcscat(Scale2, gettext(TEXT("_@M1349_")));
			break;
		default:
			break;
	}
    }

    if (ReplayLogger::IsEnabled()) {
		// LKTOKEN _@M1350_ " REPLAY"
      _tcscat(Scale2, gettext(TEXT("_@M1350_")));
    }
    if (BallastTimerActive) {
		// LKTOKEN _@M1351_ " BALLAST"
      _stprintf(TEMP,TEXT("%s %3.0fL"), gettext(TEXT("_@M1351_")), WEIGHTS[2]*BALLAST);
      _tcscat(Scale2, TEMP);
    }

    _tcscpy(Scale,TEXT(""));
    double mapScale=Units::ToSysDistance(zoom.Scale()*1.4);	// 1.4 for mapscale symbol size on map screen
    // zoom.Scale() gives user units, but FormatUserMapScale() needs system distance units
    Units::FormatUserMapScale(NULL, mapScale, Scale, sizeof(Scale)/sizeof(Scale[0]));

    SIZE tsize;

    GetTextExtentPoint(hDC, Scale, _tcslen(Scale), &tsize);
    LKWriteText(hDC, Scale, rc.right-NIBLSCALE(11)-tsize.cx, End.y+NIBLSCALE(3), 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true); 

    GetTextExtentPoint(hDC, Scale2, _tcslen(Scale2), &tsize);
    COLORREF mapscalecolor=OverColorRef;
    if (!CALCULATED_INFO.TerrainValid) 
	if (terrainwarning>0 && terrainwarning<120) mapscalecolor=RGB_RED;
		
    LKWriteText(hDC, Scale2, rc.right-NIBLSCALE(11)-tsize.cx, End.y+NIBLSCALE(3)+tsize.cy, 
	0, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true); 


    // restore original color
    SetTextColor(hDC, origcolor);

    SelectObject(hDC, hpOld);


}

