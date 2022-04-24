/*
 * navbox1 , row1 aka bottom row
 * temporary
 */

Surface.SelectObject(LK8BottomBarValueFont);
Surface.GetTextSize(BufferValue, &TextSize);

const LKIcon* pBmpTemp = GetDrawBmpIcon(bmpValue);
if (pBmpTemp) {
    pBmpTemp->Draw(Surface, rcx - (TextSize.cy/2) - NIBLSCALE(1), yRow1Value - (TextSize.cy/2) + NIBLSCALE(1), TextSize.cy - NIBLSCALE(2), TextSize.cy - NIBLSCALE(2));
}
else {
  if (showunit) {
    LKWriteText(Surface, BufferValue, rcx, yRow1Value, WTMODE_NORMAL,
                WTALIGN_CENTER, barTextColor, false);
  } else {
    LKWriteText(Surface, BufferValue, rcx, yRow1Value, WTMODE_NORMAL,
                WTALIGN_CENTER, IsDithered() ? RGB_WHITE : RGB_AMBER, false);
  }
  if (showunit && !HideUnits) {
    Surface.SelectObject(LK8BottomBarUnitFont);
    LKWriteText(Surface, BufferUnit, rcx + (TextSize.cx / 2) + NIBLSCALE(1),
                yRow1Unit, WTMODE_NORMAL, WTALIGN_LEFT, barTextColor, false);
  }
}
Surface.SelectObject(LK8BottomBarTitleFont);
rcy = yRow1Title;
