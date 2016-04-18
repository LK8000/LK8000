/*
 * navbox2 - (top row, aka row2)
 * temporary
 * second raw for portrait
 */


  Surface.SelectObject(LK8BottomBarValueFont);
  Surface.GetTextSize(BufferValue, &TextSize);
  if (showunit==true)
	LKWriteText(Surface, BufferValue, rcx, yRow2Value, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);
  else
    #ifdef DITHER
	LKWriteText(Surface, BufferValue, rcx, yRow2Value, WTMODE_NORMAL,WTALIGN_CENTER,RGB_WHITE, false);
	#else
	LKWriteText(Surface, BufferValue, rcx, yRow2Value, WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER, false);
	#endif

  if (showunit==true && !HideUnits) {
	Surface.SelectObject(LK8BottomBarUnitFont);
	LKWriteText(Surface, BufferUnit, rcx+(TextSize.cx/2)+NIBLSCALE(1), yRow2Unit, WTMODE_NORMAL, WTALIGN_LEFT,barTextColor, false);
  }

  Surface.SelectObject(LK8BottomBarTitleFont);
  rcy=yRow2Title;

