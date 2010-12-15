/*
 * navbox2 - (top row, aka row2)
 * temporary
 * second raw for portrait
 */


  SelectObject(hdc, LK8ValueFont);
  GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
  if (showunit==true)
	LKWriteText(hdc, BufferValue, rcx, yRow2Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE, false);
  else
	LKWriteText(hdc, BufferValue, rcx, yRow2Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);

  if (showunit==true && !HideUnits) {
	SelectObject(hdc, LK8UnitFont);
	LKWriteText(hdc, BufferUnit, rcx+(TextSize.cx/2)+NIBLSCALE(1), yRow2Unit , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITE, false);
  }

  SelectObject(hdc, LK8TitleNavboxFont);
  rcy=yRow2Title;

