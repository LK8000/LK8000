/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "MapWindow.h"
#include "RGB.h"

#define WPCIRCLESIZE        2 // check also duplicate in LKDrawWaypoint


bool TextInBoxMoveInView(POINT *offset, RECT *brect){

  bool res = false;

  int LabelMargin = 4;

  offset->x = 0;
  offset->y = 0;

  if (MapWindow::MapRect.top > brect->top){
    int d = MapWindow::MapRect.top - brect->top;
    brect->top += d;
    brect->bottom += d;
    offset->y += d;
    brect->bottom -= d;
    brect->left -= d;
    offset->x -= d;
    res = true;
  }

  if (MapWindow::MapRect.right < brect->right){
    int d = MapWindow::MapRect.right - brect->right;

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

  if (MapWindow::MapRect.bottom < brect->bottom){
    if (offset->x == 0){
      int d = MapWindow::MapRect.bottom - brect->bottom;
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

  if (MapWindow::MapRect.left > brect->left){
    int d = MapWindow::MapRect.left - brect->left;
    brect->right+= d;
    brect->left += d;
    offset->x += d;
    res = true;
  }

  return(res);

}



bool MapWindow::TextInBox(HDC hDC, TCHAR* Value, int x, int y, 
                          int size, TextInBoxMode_t Mode, bool noOverlap) {

  SIZE tsize;
  RECT brect;
  HFONT oldFont=0;
  bool drawn=false;

  if ((x<MapRect.left-WPCIRCLESIZE) || 
      (x>MapRect.right+(WPCIRCLESIZE*3)) || 
      (y<MapRect.top-WPCIRCLESIZE) ||
      (y>MapRect.bottom+WPCIRCLESIZE)) {
    return drawn; // FIX Not drawn really
  }

  if (size==0) {
    size = _tcslen(Value);
  }
  
  HBRUSH hbOld;
  hbOld = (HBRUSH)SelectObject(hDC, GetStockObject(WHITE_BRUSH));

  if (Mode.AsFlag.Reachable){
    if (Appearance.IndLandable == wpLandableDefault){
      x += 5;  // make space for the green circle
    }else
      if (Appearance.IndLandable == wpLandableAltA){
	x += 0;
      }
  }

  // landable waypoint label inside white box 
  if (!Mode.AsFlag.NoSetFont) {  // VENTA5 predefined font from calling function
    if (Mode.AsFlag.Border){
      oldFont = (HFONT)SelectObject(hDC, MapWindowBoldFont);
    } else {
      oldFont = (HFONT)SelectObject(hDC, MapWindowFont);
    }
  }
  
  GetTextExtentPoint(hDC, Value, size, &tsize);

  if (Mode.AsFlag.AlligneRight){
    x -= tsize.cx;
  } else 
    if (Mode.AsFlag.AlligneCenter){
      x -= tsize.cx/2;
      y -= tsize.cy/2;
    }

  bool notoverlapping = true;

  if (Mode.AsFlag.Border || Mode.AsFlag.WhiteBorder){

    POINT offset;

    brect.left = x-2;
    brect.right = brect.left+tsize.cx+4;
    brect.top = y+((tsize.cy+4)>>3)-2;
    brect.bottom = brect.top+3+tsize.cy-((tsize.cy+4)>>3);

    if (Mode.AsFlag.AlligneRight)
      x -= 3;

    if (TextInBoxMoveInView(&offset, &brect)){
      x += offset.x;
      y += offset.y;
    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif

  
    if (!noOverlap || notoverlapping) {
      HPEN oldPen;
      if (Mode.AsFlag.Border) {
        oldPen = (HPEN)SelectObject(hDC, hpMapScale);
      } else {
        oldPen = (HPEN)SelectObject(hDC, GetStockObject(WHITE_PEN));
      }
      RoundRect(hDC, brect.left, brect.top, brect.right, brect.bottom, 
                NIBLSCALE(8), NIBLSCALE(8));
      SelectObject(hDC, oldPen);
	  if (Mode.AsFlag.SetTextColor) TextColor(hDC,Mode.AsFlag.Color); else TextColor(hDC, TEXTBLACK);
#if (WINDOWSPC>0)
      SetBkMode(hDC,TRANSPARENT);
      ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
#else
      ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
#endif
      drawn=true;
    }


  } else if (Mode.AsFlag.FillBackground) {

    POINT offset;

    brect.left = x-1;
    brect.right = brect.left+tsize.cx+1;  
    brect.top = y+((tsize.cy+4)>>3);
    brect.bottom = brect.top+tsize.cy-((tsize.cy+4)>>3);

    if (Mode.AsFlag.AlligneRight)
      x -= 2;

    if (TextInBoxMoveInView(&offset, &brect)){
      x += offset.x;
      y += offset.y;
    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif
  
    if (!noOverlap || notoverlapping) {
      COLORREF oldColor = SetBkColor(hDC, RGB_WHITE);
      ExtTextOut(hDC, x, y, ETO_OPAQUE, &brect, Value, size, NULL);
      SetBkColor(hDC, oldColor);
      drawn=true;
    }

  } else if (Mode.AsFlag.WhiteBold) {

    switch (DeclutterMode) {
	// This is duplicated later on!
	case (DeclutterMode_t)dmVeryHigh:
	    brect.left = x-NIBLSCALE(10);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(10);
	    brect.top = y+((tsize.cy+NIBLSCALE(12))>>3)-NIBLSCALE(12);
	    brect.bottom = brect.top+NIBLSCALE(12)+tsize.cy-((tsize.cy+NIBLSCALE(12))>>3);
	    break;
	case (DeclutterMode_t)dmHigh:
	    brect.left = x-NIBLSCALE(5);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(5);
	    brect.top = y+((tsize.cy+NIBLSCALE(6))>>3)-NIBLSCALE(6);
	    brect.bottom = brect.top+NIBLSCALE(6)+tsize.cy-((tsize.cy+NIBLSCALE(6))>>3);
	    break;
	case (DeclutterMode_t)dmMedium:
	    brect.left = x-NIBLSCALE(2);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	    brect.top = y+((tsize.cy+NIBLSCALE(3))>>3)-NIBLSCALE(3);
	    brect.bottom = brect.top+NIBLSCALE(3)+tsize.cy-((tsize.cy+NIBLSCALE(3))>>3);
	    break;
	case (DeclutterMode_t)dmLow:
	case (DeclutterMode_t)dmDisabled: // BUGFIX 100909
	    brect.left = x;
	    brect.right = brect.left+tsize.cx;
	    brect.top = y+((tsize.cy)>>3);
	    brect.bottom = brect.top+tsize.cy-((tsize.cy)>>3);
	    break;
	default:
	    break;

    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif
  
    if (!noOverlap || notoverlapping) { 
      if (OutlinedTp)
	SetTextColor(hDC,RGB_BLACK);
      else
	SetTextColor(hDC,RGB_WHITE); 

#ifdef WINE
      SetBkMode(hDC,TRANSPARENT);
      ExtTextOut(hDC, x+2, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x+1, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-1, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-2, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y+1, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y-1, 0, NULL, Value, size, NULL);
#else /* WINE */
      ExtTextOut(hDC, x+2, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x+1, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-1, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-2, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y+1, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y-1, ETO_OPAQUE, NULL, Value, size, NULL);
#endif /* WINE */

      if (OutlinedTp) {
	TextColor(hDC,Mode.AsFlag.Color);
      } else
	SetTextColor(hDC,RGB_BLACK); 

#ifdef WINE
      ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
#else
      ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
#endif /* WINE */
      if (OutlinedTp)
	SetTextColor(hDC,RGB_BLACK); // TODO somewhere else text color is not set correctly
      drawn=true;
    }

  } else {

    switch (DeclutterMode) {
	// This is duplicated before!
	case (DeclutterMode_t)dmVeryHigh:
	    brect.left = x-NIBLSCALE(10);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(10);
	    brect.top = y+((tsize.cy+NIBLSCALE(12))>>3)-NIBLSCALE(12);
	    brect.bottom = brect.top+NIBLSCALE(12)+tsize.cy-((tsize.cy+NIBLSCALE(12))>>3);
	    break;
	case (DeclutterMode_t)dmHigh:
	    brect.left = x-NIBLSCALE(5);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(5);
	    brect.top = y+((tsize.cy+NIBLSCALE(6))>>3)-NIBLSCALE(6);
	    brect.bottom = brect.top+NIBLSCALE(6)+tsize.cy-((tsize.cy+NIBLSCALE(6))>>3);
	    break;
	case (DeclutterMode_t)dmMedium:
	    brect.left = x-NIBLSCALE(2);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	    brect.top = y+((tsize.cy+NIBLSCALE(3))>>3)-NIBLSCALE(3);
	    brect.bottom = brect.top+NIBLSCALE(3)+tsize.cy-((tsize.cy+NIBLSCALE(3))>>3);
	    break;
	case (DeclutterMode_t)dmLow:
	case (DeclutterMode_t)dmDisabled: // BUGFIX 100909
	    brect.left = x;
	    brect.right = brect.left+tsize.cx;
	    brect.top = y+((tsize.cy)>>3);
	    brect.bottom = brect.top+tsize.cy-((tsize.cy)>>3);
	    break;
	default:
	    break;

    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif
  
    if (!noOverlap || notoverlapping) {
#if (WINDOWSPC>0)
      SetBkMode(hDC,TRANSPARENT);
		TextColor(hDC,Mode.AsFlag.Color);
      		ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
		SetTextColor(hDC,RGB_BLACK); 
#else
		TextColor(hDC,Mode.AsFlag.Color);
      		ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
		SetTextColor(hDC,RGB_BLACK); 
#endif
      drawn=true;
    }

  }
 
  if (!Mode.AsFlag.NoSetFont) SelectObject(hDC, oldFont); // VENTA5
  SelectObject(hDC, hbOld);

  return drawn;

}



#if TOPOFASTLABEL

int MapWindow::nLabelBlocks;
int MapWindow::nVLabelBlocks[SCREENVSLOTS+1];
RECT MapWindow::LabelBlockCoords[SCREENVSLOTS+1][MAXVLABELBLOCKS+1];

// this slots char array is simply loading the slot number. 
// A nibble should be enough, but no problems to use 8 bits.
char * MapWindow::slot;

//
// Returns true if label can be printed, not overlapping other labels
//
bool MapWindow::checkLabelBlock(RECT *rc) {
  static bool doinit=true;

  // This item is out of screen, probably because zoom was made and we still have old wps
  // or we panned, or we have a far away takeoff still in the list
  if (rc->top <0 || rc->top>ScreenSizeY) return false;
  // we must limit the out of screen of bottom label size to clipped screen
  if (rc->bottom>ScreenSizeY) rc->bottom=ScreenSizeY;

  if (rc->left>ScreenSizeX) return false;
  if (rc->right<0) return false;

  if (doinit) {
	// vertical coordinate Y for bottom size of each slot
	unsigned int slotbottom[SCREENVSLOTS+1];
	unsigned int slotsize=ScreenSizeY/SCREENVSLOTS;
	unsigned int i, j;
	for (j=0; j<(SCREENVSLOTS-1); j++) {
		i=(j*slotsize)+slotsize;
		slotbottom[j]=i;
	}
	slotbottom[SCREENVSLOTS-1]=ScreenSizeY;

	slot=(char *)malloc((ScreenSizeY+1)*sizeof(char));
	// j initially is slot 0; we keep <= for safety
	for (i=0, j=0; i<=(unsigned int)ScreenSizeY; i++) {
		if ( i>slotbottom[j] ) j++;
		// just for safety
		if (j>(SCREENVSLOTS-1)) j=SCREENVSLOTS-1;
		slot[i]=(char)j;
	}

	doinit=false;
  }

  if (DeclutterMode==(DeclutterMode_t)dmDisabled) return true;

  // Max number of labels on screen
  if (nLabelBlocks>LKMaxLabels) return false;

  // rc.top is searched in its slot, but the label could also spread to the next slot...

  unsigned int vslot=(char)slot[rc->top];

  #define nvlabelslot nVLabelBlocks[vslot]

  // Check rc.top in its slot
  for (int i=0; i< nvlabelslot; i++) {
	// CheckRect is used only here
	if (CheckRectOverlap(&LabelBlockCoords[vslot][i],rc)) {
		// When overlapping, DO NOT insert this label in the list! It has not been printed!
		// StartupStore(_T("... item %d overlapping in slot %d with nvlabels=%d\n"),i,vslot,nvlabelslot);
		return false;
	}
  }
  // top is ok, now check if using also next slot
  bool doslot2=false;
  unsigned int v2slot=(char)slot[rc->bottom];
  #define nv2labelslot nVLabelBlocks[v2slot]
  if (v2slot != vslot) {
	for (int i=0; i< nv2labelslot; i++) {
		//if (CheckRectOverlap(&LabelBlockCoords[v2slot][i],&rc)) {
		if (CheckRectOverlap(&LabelBlockCoords[v2slot][i],rc)) {
			// StartupStore(_T("... item %d overlapping in secondary slot %d with nvlabels=%d\n"),i,v2slot,nv2labelslot);
			return false;
		}
	}
	doslot2=true;
  }

  // now insert the label in the list, for next checks
  if (nvlabelslot <(MAXVLABELBLOCKS-1)) {
	LabelBlockCoords[vslot][nvlabelslot]= *rc;
	nLabelBlocks++;
	nVLabelBlocks[vslot]++;
	// StartupStore(_T("... added label in slot %d nvlabelslot now=%d tot=%d\n"), vslot,nVLabelBlocks[vslot], nLabelBlocks);
	if (!doslot2) return true;
  } else {
	// if the label cannot be checked because the list is full, don't print the label!
	// StartupStore(_T("... label list is full vslot=%d, item not added%s"),vslot,NEWLINE);
	return false;
  }

  // Now check secondary list, if needed
  if (nv2labelslot <(MAXVLABELBLOCKS-1)) {
	LabelBlockCoords[v2slot][nv2labelslot]= *rc;
	nLabelBlocks++;
	nVLabelBlocks[v2slot]++;
	// StartupStore(_T("... added label in slot %d nvlabelslot now=%d tot=%d\n"), vslot,nVLabelBlocks[vslot], nLabelBlocks);
	return true;
  } else {
	// if the label cannot be checked because the list is full, don't print the label!
	// StartupStore(_T("... second label list is full v2slot=%d, item not added%s"),v2slot,NEWLINE);
	return false;
  }

  return true;
}

#endif

