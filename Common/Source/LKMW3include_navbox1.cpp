/*
 * navbox1 , row1 aka bottom row
 * temporary
 */


  Surface.SelectObject(LK8BottomBarValueFont);
  Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
  if (showunit==true)
	LKWriteText(Surface, BufferValue, rcx, yRow1Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);
  else
	LKWriteText(Surface, BufferValue, rcx, yRow1Value, 0, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);

  if (showunit==true && !HideUnits) {
	Surface.SelectObject(LK8UnitFont);
	LKWriteText(Surface, BufferUnit, rcx+(TextSize.cx/2)+NIBLSCALE(1), yRow1Unit , 0, WTMODE_NORMAL, WTALIGN_LEFT,barTextColor, false);
  }
  Surface.SelectObject(LK8BottomBarTitleFont);
  rcy=yRow1Title;

