
void DrawTelescope(HDC hdc, double fAngle, int x, int y)
{
	POINT Telescope[17] = {
			{  6 ,  7  },    // 1
			{  6 ,  2  },    // 2
			{  8 , -2  },    // 3
			{  8 , -7  },    // 4
			{  1 , -7  },    // 5
			{  1 , -2  },    // 6
			{ -1 , -2  },    // 7
			{ -1 , -7  },    // 8
			{ -8 , -7  },    // 9
			{ -8 , -2  },    // 10
			{ -6 ,  2  },    // 11
			{ -6 ,  7  },    // 12
			{ -1 ,  7  },    // 13
			{ -1 ,  3  },    // 14
			{  1 ,  3  },    // 15
			{  1 ,  7  },    // 16
			{  4 ,  7  }     // 17
	};



bool bBlack = true;
DrawWindRoseDirection( hdc, AngleLimit360( fAngle ),  x,  y + NIBLSCALE(18));
PolygonRotateShift(Telescope, 17, x, y, AngleLimit360( fAngle  ));

LKPen	oldBPen ;
LKBrush oldBrush ;
if (!bBlack)
{
  oldBPen  = Surface.SelectObject(LK_WHITE_PEN));
  oldBrush = Surface.SelectObject(LKBrush_White);
}
else
{
  oldBPen  = Surface.SelectObject(LK_BLACK_PEN);
  oldBrush = Surface.SelectObject(LKBrush_Black);
}
Surface.Polygon(Telescope,17);

if (!bBlack)
  Surface.SelectObject(LK_BLACK_PEN));
else
  Surface.SelectObject(LK_WHITE_PEN));

Surface.Polygon(Telescope,17);

Surface.SelectObject(oldBrush);
Surface.SelectObject(oldBPen);
}




void RenderBearingDiff(HDC hdc,double brg, DiagrammStruct* psDia )
{
RECT rc	= psDia->rc;
  // Print Bearing difference
  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];
  TCHAR szOvtname[80];
  GetOvertargetName(szOvtname);

  bool ret = false;
  // Borrowed from LKDrawLook8000.cpp
  switch (OvertargetMode) {
    case OVT_TASK:
      // Do not use FormatBrgDiff for TASK, could be AAT!
      ret = MapWindow::LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
      break;
    case OVT_ALT1:
      MapWindow::LKFormatBrgDiff(Alternate1, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_ALT2:
      MapWindow::LKFormatBrgDiff(Alternate2, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_BALT:
      MapWindow::LKFormatBrgDiff(BestAlternate, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_THER:
      MapWindow::LKFormatBrgDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_HOME:
      MapWindow::LKFormatBrgDiff(HomeWaypoint, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_MATE:
      MapWindow::LKFormatBrgDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_FLARM:
      MapWindow::LKFormatBrgDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
      ret = true;
      break;
    default:
      ret = MapWindow::LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
      break;
  }


  SIZE tsize;
  int x = (rc.right + rc.left)/2;
  int y = rc.top+35;

  if (ret) {
    Surface.SelectObject(LK8MediumFont);
    GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &tsize);
    y = rc.top;
    DrawText(hdc, x- tsize.cx/2, y, ETO_OPAQUE, NULL, BufferValue, _tcslen(BufferValue), NULL);

	y += tsize.cy-5;
	Surface.SelectObject(LK8PanelUnitFont);
	GetTextExtentPoint(hdc, szOvtname, _tcslen(szOvtname), &tsize);
	DrawText(hdc, x-tsize.cx/2, y, ETO_OPAQUE, NULL, szOvtname, _tcslen(szOvtname), NULL);
  }
}


