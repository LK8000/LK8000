/*
 * navbox1 , row1 aka bottom row
 * temporary
 */


  SelectObject(hdc, LK8ValueFont);
  GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
  if (showunit==true)
	LKWriteText(hdc, BufferValue, rcx, yRow1Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);
  else
	LKWriteText(hdc, BufferValue, rcx, yRow1Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);

  if (showunit==true && !HideUnits) {
	SelectObject(hdc, LK8UnitFont);
	LKWriteText(hdc, BufferUnit, rcx+(TextSize.cx/2)+NIBLSCALE(1), yRow1Unit , 0, WTMODE_NORMAL, WTALIGN_LEFT,barTextColor, false);
  }
  SelectObject(hdc, LK8TitleNavboxFont);
  rcy=yRow1Title;

