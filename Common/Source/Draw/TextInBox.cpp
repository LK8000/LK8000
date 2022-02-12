/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "DoInits.h"
#include "LKObjects.h"
#include "ScreenGeometry.h"


bool TextInBoxMoveInView(const RECT *clipRect, POINT *offset, RECT *brect){

  bool res = false;

  int LabelMargin = 4;

  offset->x = 0;
  offset->y = 0;

  if (clipRect->top > brect->top){
    int d = clipRect->top - brect->top;
    brect->top += d;
    brect->bottom += d;
    offset->y += d;
    brect->bottom -= d;
    brect->left -= d;
    offset->x -= d;
    res = true;
  }

  if (clipRect->right < brect->right){
    int d = clipRect->right - brect->right;

    if (offset->y < LabelMargin){
      int dy;

      if (d > -LabelMargin){
        dy = LabelMargin-offset->y;
        if (d > -dy)
          dy = -d;
      } else {
        int x = d + (brect->right - brect->left) + 10;

        dy = x - offset->y;

        if (dy < 0)
          dy = 0;

        if (dy > LabelMargin)
          dy = LabelMargin;
      }

      brect->top += dy;
      brect->bottom += dy;
      offset->y += dy;

    }

    brect->right += d;
    brect->left += d;
    offset->x += d;

    res = true;
  }

  if (clipRect->bottom < brect->bottom){
    if (offset->x == 0){
      int d = clipRect->bottom - brect->bottom;
      brect->top += d;
      brect->bottom += d;
      offset->y += d;
    } else
      if (offset->x < -LabelMargin){
	int d = -(brect->bottom - brect->top) - 10;
	brect->top += d;
	brect->bottom += d;
	offset->y += d;
      } else {
	int d = -(2*offset->x + (brect->bottom - brect->top));
	brect->top += d;
	brect->bottom += d;
	offset->y += d;
      }

    res = true;
  }

  if (clipRect->left > brect->left){
    int d = clipRect->left - brect->left;
    brect->right+= d;
    brect->left += d;
    offset->x += d;
    res = true;
  }

  return(res);

}



bool MapWindow::TextInBox(LKSurface& Surface, const RECT *clipRect,  const TCHAR* Value, int x, int y,
                          TextInBoxMode_t *Mode, bool noOverlap) {

  SIZE tsize;
  RECT brect;
  LKSurface::OldFont oldFont {};
  bool drawn=false;

  if ((x<clipRect->left-WPCIRCLESIZE) ||
      (x>clipRect->right+(WPCIRCLESIZE*3)) ||
      (y<clipRect->top-WPCIRCLESIZE) ||
      (y>clipRect->bottom+WPCIRCLESIZE)) {
    return drawn;
  }

  if (Mode == NULL) return false;

  const auto hbOld = Surface.SelectObject(LKBrush_White);
  const auto hpOld = Surface.SelectObject(LK_BLACK_PEN);

  if (Mode->Reachable){
    if (Appearance.IndLandable == wpLandableDefault){
      x += 5;  // make space for the green circle
    }else
      if (Appearance.IndLandable == wpLandableAltA){
	x += 0;
      }
  }

  // landable waypoint label inside white box
  if (!Mode->NoSetFont) {
    if (Mode->Border || Mode->WhiteBold){
      oldFont = Surface.SelectObject(MapWaypointBoldFont);
    } else {
      oldFont = Surface.SelectObject(MapWaypointFont);
    }
  }

  Surface.GetTextSize(Value, &tsize);

  if (Mode->AlligneRight){
    x -= tsize.cx;
  } else
    if (Mode->AlligneCenter){
      x -= tsize.cx/2;
      y -= tsize.cy/2;
    }

  if (Mode->Border || Mode->WhiteBorder){

    POINT offset;

    brect.left = x-2;
    brect.right = brect.left+tsize.cx+4;
    brect.top = y+((tsize.cy+4)>>3)-2;
    brect.bottom = brect.top+3+tsize.cy-((tsize.cy+4)>>3);

    if (Mode->AlligneRight)
      x -= 3;

    if (TextInBoxMoveInView(clipRect, &offset, &brect)){
      x += offset.x;
      y += offset.y;
    }

    if (!noOverlap || checkLabelBlock(brect, *clipRect)) {
      LKSurface::OldPen oldPen;
      if (Mode->Border) {
        oldPen = Surface.SelectObject(LKPen_Black_N1);
      } else {
        oldPen = Surface.SelectObject(LK_WHITE_PEN);
      }
      Surface.RoundRect(brect, NIBLSCALE(4), NIBLSCALE(4));
      Surface.SelectObject(oldPen);
      if (Mode->SetTextColor) {
        Surface.SetTextColor(Mode->Color); 
      } else {
        Surface.SetTextColor(RGB_BLACK);
      } 

      Surface.SetBackgroundTransparent();
      
#ifndef __linux__
      Surface.DrawText(x, y, Value);
#else
      Surface.DrawText(x, y+NIBLSCALE(1), Value);
#endif
      drawn=true;
    }


  } else if (Mode->FillBackground) {

    POINT offset;

    brect.left = x-1;
    brect.right = brect.left+tsize.cx+1;
    brect.top = y+((tsize.cy+4)>>3);
    brect.bottom = brect.top+tsize.cy-((tsize.cy+4)>>3);

    if (Mode->AlligneRight)
      x -= 2;

    if (TextInBoxMoveInView(clipRect, &offset, &brect)){
      x += offset.x;
      y += offset.y;
    }

    if (!noOverlap || checkLabelBlock(brect, *clipRect)) {
      LKColor oldColor = Surface.SetBkColor(RGB_WHITE);
      Surface.DrawText(x, y, Value);
      Surface.SetBkColor(oldColor);
      drawn=true;
    }

  } else if (Mode->WhiteBold) {

    switch (DeclutterMode) {
	// This is duplicated later on!
	case dmVeryHigh:
	    brect.left = x-NIBLSCALE(10);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(10);
	    brect.top = y+((tsize.cy+NIBLSCALE(12))>>3)-NIBLSCALE(12);
	    brect.bottom = brect.top+NIBLSCALE(12)+tsize.cy-((tsize.cy+NIBLSCALE(12))>>3);
	    break;
	case dmHigh:
	    brect.left = x-NIBLSCALE(5);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(5);
	    brect.top = y+((tsize.cy+NIBLSCALE(6))>>3)-NIBLSCALE(6);
	    brect.bottom = brect.top+NIBLSCALE(6)+tsize.cy-((tsize.cy+NIBLSCALE(6))>>3);
	    break;
	case dmMedium:
	    brect.left = x-NIBLSCALE(2);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	    brect.top = y+((tsize.cy+NIBLSCALE(3))>>3)-NIBLSCALE(3);
	    brect.bottom = brect.top+NIBLSCALE(3)+tsize.cy-((tsize.cy+NIBLSCALE(3))>>3);
	    break;
	case dmLow:
	case dmDisabled: // BUGFIX 100909
	    brect.left = x;
	    brect.right = brect.left+tsize.cx;
	    brect.top = y+((tsize.cy)>>3);
	    brect.bottom = brect.top+tsize.cy-((tsize.cy)>>3);
	    break;
	default:
	    break;

    }

	if (!noOverlap || checkLabelBlock(brect, *clipRect)) {
      if (OutlinedTp)
	Surface.SetTextColor(RGB_BLACK);
      else
	Surface.SetTextColor(RGB_WHITE);


    #ifdef WINE
    Surface.SetBackgroundTransparent();
    #endif
    // Simplified, shadowing better and faster
    // ETO_OPAQUE not necessary since we pass a NULL rect
    //

#if !defined(PNA) || defined(UNDER_CE)
    short emboldsize=IBLSCALE(1);
    for (short a=1; a<=emboldsize; a++) {
       Surface.DrawText(x-a, y-a, Value);
       Surface.DrawText(x-a, y+a, Value);
       Surface.DrawText(x+a, y-a, Value);
       Surface.DrawText(x+a, y+a, Value);
    }
    if (OutlinedTp) {
        short a=emboldsize+1;
        Surface.DrawText(x-a, y, Value);
        Surface.DrawText(x+a, y, Value);
        Surface.DrawText(x, y-a, Value);
        Surface.DrawText(x, y+a, Value);
    }
#else
    Surface.DrawText(x-1, y-1, Value);
    Surface.DrawText(x-1, y+1, Value);
    Surface.DrawText(x+1, y-1, Value);
    Surface.DrawText(x+1, y+1, Value);

    if (OutlinedTp) {
        Surface.DrawText(x-2, y, Value);
        Surface.DrawText(x+2, y, Value);
        Surface.DrawText(x, y-2, Value);
        Surface.DrawText(x, y+2, Value);
    }
#endif

      if (OutlinedTp) {
        Surface.SetTextColor(Mode->Color);
      } else {
        Surface.SetTextColor(RGB_BLACK);
      }
      Surface.DrawText(x, y, Value);

      if (OutlinedTp)
        Surface.SetTextColor(RGB_BLACK); // TODO somewhere else text color is not set correctly
      drawn=true;
    }

  } else {

    switch (DeclutterMode) {
	// This is duplicated before!
	case dmVeryHigh:
	    brect.left = x-NIBLSCALE(10);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(10);
	    brect.top = y+((tsize.cy+NIBLSCALE(12))>>3)-NIBLSCALE(12);
	    brect.bottom = brect.top+NIBLSCALE(12)+tsize.cy-((tsize.cy+NIBLSCALE(12))>>3);
	    break;
	case dmHigh:
	    brect.left = x-NIBLSCALE(5);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(5);
	    brect.top = y+((tsize.cy+NIBLSCALE(6))>>3)-NIBLSCALE(6);
	    brect.bottom = brect.top+NIBLSCALE(6)+tsize.cy-((tsize.cy+NIBLSCALE(6))>>3);
	    break;
	case dmMedium:
	    brect.left = x-NIBLSCALE(2);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	    brect.top = y+((tsize.cy+NIBLSCALE(3))>>3)-NIBLSCALE(3);
	    brect.bottom = brect.top+NIBLSCALE(3)+tsize.cy-((tsize.cy+NIBLSCALE(3))>>3);
	    break;
	case dmLow:
	case dmDisabled: // BUGFIX 100909
	    brect.left = x;
	    brect.right = brect.left+tsize.cx;
	    brect.top = y+((tsize.cy)>>3);
	    brect.bottom = brect.top+tsize.cy-((tsize.cy)>>3);
	    break;
	default:
	    break;

    }

    if (!noOverlap || checkLabelBlock(brect, *clipRect)) {
      Surface.SetTextColor(Mode->Color);
      Surface.DrawText(x, y, Value);
      Surface.SetTextColor(RGB_BLACK);
      drawn=true;
    }

  }

  if (oldFont) Surface.SelectObject(oldFont); // VENTA5
  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);

  return drawn;

}




int MapWindow::nLabelBlocks;
MapWindow::LabelBlockCoords_t MapWindow::LabelBlockCoords;

/**
 * Returns true if label can be printed, not overlapping other labels
 */
bool MapWindow::checkLabelBlock(const RECT& rc, const RECT& clipRect) {

  // Declutter is disabled, no need to check overlap
  if (DeclutterMode == dmDisabled) {
    return true;
  }

  // Max number of labels on screen
  if (nLabelBlocks > LKMaxLabels) {
    return false;
  }

  // empty rect
  if(rc.top >= rc.bottom || rc.left >= rc.right) {
    return false;
  }

  // do not draw incomplete label to let the place available for fully visible one.
  if (rc.right > clipRect.right || rc.left < clipRect.left)  {
    // todo : do not exclude if visible part is long enought ?
    //    rules could be "visible_width > (height * 5)" or "visible_width > (total_width * 2/3)"
    return false;
  }
  if (rc.bottom > clipRect.bottom || rc.top < clipRect.top)  {
    return false;
  }

  const int slotsize = std::max<int>(ScreenSizeY / (LabelBlockCoords.size() - 1), 1);

  const unsigned vslot_top = std::max<unsigned>(rc.top / slotsize, 0); // first slot index
  const unsigned vslot_bot = std::min<unsigned>(rc.bottom / slotsize, LabelBlockCoords.size() - 1); // last slot index

  assert(vslot_top <= vslot_bot); // integer overflow ?

  // check overlap
  for (unsigned i = vslot_top; i <= vslot_bot; ++i) {
    const auto& slot = LabelBlockCoords[i];
    for (const auto& Rect : slot) {
      if (CheckRectOverlap(&rc, &Rect)) {
        // When overlapping, DO NOT insert this label in the list! It has not been printed!
        return false;
      }
    }
  }

  try {
    // now insert the label Rect in the list, for next checks
    for (unsigned i = vslot_top; i <= vslot_bot; ++i) {
      LabelBlockCoords[i].emplace_back(rc);
    }
    nLabelBlocks++;
  } catch (const std::exception& e) {
    /*
     * std::vector::emplace_back can throw exception if the allocation 
     * request does not succeed (not enough memory)
     */
    assert(false);
    StartupStore(_T("MapWindow::checkLabelBlock : %s\n"), to_tstring(e.what()).c_str());
    return false;
  }
  return true;
}

/**
 * Called by Thread_Draw, to init
 * Called by RenderMapWindowBg at runtime
 *  also called by main thread when screen size change
 *  all use are already protected by #Surface_Mutex, no need another mutex.
 */
void MapWindow::ResetLabelDeclutter() {
  nLabelBlocks = 0;
  for(auto& blocks : LabelBlockCoords) {
    blocks.clear();
  }
}

/**
 * This force temporarily no labels to be printed, by saturating the declutter.
 *  20/09/2017 : not Used.
 */
void MapWindow::SaturateLabelDeclutter() {
  nLabelBlocks = LKMaxLabels+1;
}
