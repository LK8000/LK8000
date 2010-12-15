/*
 * navbox1 , row1 aka bottom row
 * temporary
 */


  #if NEWPNAV

  SelectObject(hdc, LK8ValueFont);
  GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
  if (showunit==true)
	LKWriteText(hdc, BufferValue, rcx, yRow1Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE, false);
  else
	LKWriteText(hdc, BufferValue, rcx, yRow1Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);

  if (showunit==true && !HideUnits) {
	SelectObject(hdc, LK8UnitFont);
	LKWriteText(hdc, BufferUnit, rcx+(TextSize.cx/2)+NIBLSCALE(1), yRow1Unit , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITE, false);
  }
  SelectObject(hdc, LK8TitleNavboxFont);
  rcy=yRow1Title;


  #else
  SelectObject(hdc, LK8ValueFont);
  GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
  rcy=rc.bottom-(TextSize.cy/2);
  if (showunit==true)
	LKWriteText(hdc, BufferValue, rcx, rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE, false);
  else
	LKWriteText(hdc, BufferValue, rcx, rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);

  if (showunit==true && !HideUnits) {
	SelectObject(hdc, LK8UnitFont);
	LKWriteText(hdc, BufferUnit, rcx+(TextSize.cx/2), rcy-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITE, false);
  }

  SelectObject(hdc, LK8TitleNavboxFont);
  GetTextExtentPoint(hdc, BufferTitle, _tcslen(BufferTitle), &TextSize);

  #endif
