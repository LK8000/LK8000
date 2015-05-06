/*
 * navbox2 - (top row, aka row2)
 * temporary
 * second raw for portrait
 */


  Surface.SelectObject(LK8ValueFont);
  Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
  if (showunit==true)
	LKWriteText(Surface, BufferValue, rcx, yRow2Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);
  else
	LKWriteText(Surface, BufferValue, rcx, yRow2Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);

  if (showunit==true && !HideUnits) {
	Surface.SelectObject(LK8UnitFont);
	LKWriteText(Surface, BufferUnit, rcx+(TextSize.cx/2)+NIBLSCALE(1), yRow2Unit , 0, WTMODE_NORMAL, WTALIGN_LEFT,barTextColor, false);
  }

  Surface.SelectObject(LK8BottomBarTitleFont);
  rcy=yRow2Title;

