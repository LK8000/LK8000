/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "DoInits.h"
#include "LKObjects.h"

#define CLIP_TEXT	1

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
                          int size, TextInBoxMode_t *Mode, bool noOverlap) {

  SIZE tsize;
  RECT brect;
  LKFont oldFont;
  bool drawn=false;

  if ((x<clipRect->left-WPCIRCLESIZE) || 
      (x>clipRect->right+(WPCIRCLESIZE*3)) || 
      (y<clipRect->top-WPCIRCLESIZE) ||
      (y>clipRect->bottom+WPCIRCLESIZE)) {
    return drawn; 
  }

  if (Mode == NULL) return false;

  if (size==0) {
    size = _tcslen(Value);
  }
  
  LKBrush hbOld = Surface.SelectObject(LKBrush_White);
  LKPen hpOld = Surface.SelectObject(LK_BLACK_PEN);

  if (Mode->Reachable){
    if (Appearance.IndLandable == wpLandableDefault){
      x += 5;  // make space for the green circle
    }else
      if (Appearance.IndLandable == wpLandableAltA){
	x += 0;
      }
  }

  // landable waypoint label inside white box 
  if (!Mode->NoSetFont) {  // VENTA5 predefined font from calling function
    if (Mode->Border){
      oldFont = Surface.SelectObject(MapWindowBoldFont);
    } else {
      oldFont = Surface.SelectObject(MapWindowFont);
    }
  }
  
  Surface.GetTextSize(Value, size, &tsize);

  if (Mode->AlligneRight){
    x -= tsize.cx;
  } else 
    if (Mode->AlligneCenter){
      x -= tsize.cx/2;
      y -= tsize.cy/2;
    }

  bool notoverlapping = true;

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

    #if CLIP_TEXT
    if (y>=clipRect->bottom || brect.bottom>=clipRect->bottom ) return false;
    #endif

	notoverlapping = checkLabelBlock(&brect); 

  
    if (!noOverlap || notoverlapping) {
      LKPen oldPen;
      if (Mode->Border) {
        oldPen = Surface.SelectObject(LKPen_Black_N1);
      } else {
        oldPen = Surface.SelectObject(LK_WHITE_PEN);
      }
      Surface.RoundRect(brect, NIBLSCALE(8), NIBLSCALE(8));
      Surface.SelectObject(oldPen);
      if (Mode->SetTextColor) Surface.SetTextColor(Mode->Color); else Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(x, y, Value, size);
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

    #if CLIP_TEXT
    if (y>=clipRect->bottom || brect.bottom>=clipRect->bottom ) return false;
    #endif

	notoverlapping = checkLabelBlock(&brect); 
  
    if (!noOverlap || notoverlapping) {
      LKColor oldColor = Surface.SetBkColor(RGB_WHITE);
      Surface.DrawText(x, y, Value, size);
      Surface.SetBkColor(oldColor);
      drawn=true;
    }

  } else if (Mode->WhiteBold) {

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

    #if CLIP_TEXT
    if (brect.bottom >= clipRect->bottom) return false;
    #endif

	notoverlapping = checkLabelBlock(&brect); 
  
    if (!noOverlap || notoverlapping) { 
      if (OutlinedTp)
	Surface.SetTextColor(RGB_BLACK);
      else
	Surface.SetTextColor(RGB_WHITE);


    #ifdef WINE
    Surface.SetBkMode(TRANSPARENT);
    #endif
    // Simplified, shadowing better and faster
    // ETO_OPAQUE not necessary since we pass a NULL rect
    //
    Surface.DrawText(x-1, y-1, Value, size);
    Surface.DrawText(x-1, y+1, Value, size);
    Surface.DrawText(x+1, y-1, Value, size);
    Surface.DrawText(x+1, y+1, Value, size);

    if (OutlinedTp && 1) {
        Surface.DrawText(x-2, y, Value, size);
        Surface.DrawText(x+2, y, Value, size);
        Surface.DrawText(x, y-2, Value, size);
        Surface.DrawText(x, y+2, Value, size);
    }

      if (OutlinedTp) {
        Surface.SetTextColor(Mode->Color);
      } else {
        Surface.SetTextColor(RGB_BLACK);
      }
      Surface.DrawText(x, y, Value, size);

      if (OutlinedTp)
        Surface.SetTextColor(RGB_BLACK); // TODO somewhere else text color is not set correctly
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

    #if CLIP_TEXT
    if (brect.bottom >= clipRect->bottom) return false;
    #endif

	notoverlapping = checkLabelBlock(&brect); 
  
    if (!noOverlap || notoverlapping) {
      Surface.SetTextColor(Mode->Color);
      Surface.DrawText(x, y, Value, size);
      Surface.SetTextColor(RGB_BLACK);
      drawn=true;
    }

  }
 
  if (!Mode->NoSetFont) Surface.SelectObject(oldFont); // VENTA5
  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);

  return drawn;

}




int MapWindow::nLabelBlocks;
int MapWindow::nVLabelBlocks[SCREENVSLOTS+1];
RECT MapWindow::LabelBlockCoords[SCREENVSLOTS+1][MAXVLABELBLOCKS+1];

// this slots char array is simply loading the slot number. 
// A nibble should be enough, but no problems to use 8 bits.
char * MapWindow::slot=NULL;

//
// Returns true if label can be printed, not overlapping other labels
//
bool MapWindow::checkLabelBlock(RECT *rc) {

  // This item is out of screen, probably because zoom was made and we still have old wps
  // or we panned, or we have a far away takeoff still in the list
  if (rc->top <0 || rc->top>ScreenSizeY) return false;
  // we must limit the out of screen of bottom label size to clipped screen
  if (rc->bottom>ScreenSizeY) rc->bottom=ScreenSizeY;

  if (rc->left>ScreenSizeX) return false;
  if (rc->right<0) return false;

  if (DoInit[MDI_CHECKLABELBLOCK]) {
	// vertical coordinate Y for bottom size of each slot
	unsigned int slotbottom[SCREENVSLOTS+1];
	unsigned int slotsize=ScreenSizeY/SCREENVSLOTS;
	unsigned int i, j;
	for (j=0; j<(SCREENVSLOTS-1); j++) {
		i=(j*slotsize)+slotsize;
		slotbottom[j]=i;
	}
	slotbottom[SCREENVSLOTS-1]=ScreenSizeY;

	if (slot!=NULL) free(slot);

	slot=(char *)malloc((ScreenSizeY+1)*sizeof(char));
        LKASSERT(slot!=NULL);
	// j initially is slot 0; we keep <= for safety
	for (i=0, j=0; i<=(unsigned int)ScreenSizeY; i++) {
		if ( i>slotbottom[j] ) j++;
		// just for safety
		if (j>(SCREENVSLOTS-1)) j=SCREENVSLOTS-1;
		slot[i]=(char)j;
	}

	DoInit[MDI_CHECKLABELBLOCK]=false;
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

// This is used only on shutdown, to free the malloc. 
void MapWindow::FreeSlot(){
	free(slot);
	slot=NULL;
}


//
// Called by Thread_Draw, for init
// Called by RenderMapWindowBg at runtime
//
void MapWindow::ResetLabelDeclutter(void) {
  nLabelBlocks = 0;
  for (short nvi=0; nvi<SCREENVSLOTS; nvi++) nVLabelBlocks[nvi]=0;
}


//
// This will force temporarily no labels to be printed, by saturating the declutter.
// A dirty trick.
//
void MapWindow::SaturateLabelDeclutter(void) {
  for (short nvi=0; nvi<SCREENVSLOTS; nvi++) nVLabelBlocks[nvi]=MAXVLABELBLOCKS;
}

#if 0
//
// Adjust icon rect area according to desired decluttering level
//
void MapWindow::SetDeclutterIcon(RECT *drect) {

  switch (DeclutterMode) {
	case (DeclutterMode_t)dmDisabled:
		break;
	case (DeclutterMode_t)dmLow:
		drect->left -= 1;
		drect->right += 1;
		drect->top -= NIBLSCALE(1);
		drect->bottom += NIBLSCALE(1);
		break;
	case (DeclutterMode_t)dmMedium:
		drect->left -= NIBLSCALE(2);
		drect->right += NIBLSCALE(2);
		drect->top -= NIBLSCALE(3);
		drect->bottom += NIBLSCALE(3);
		break;
	case (DeclutterMode_t)dmHigh:
		drect->left -= NIBLSCALE(5);
		drect->right += NIBLSCALE(5);
		drect->top -= NIBLSCALE(6);
		drect->bottom += NIBLSCALE(6);
		break;
	case (DeclutterMode_t)dmVeryHigh:
		drect->left -= NIBLSCALE(10);
		drect->right += NIBLSCALE(10);
		drect->top -= NIBLSCALE(12);
		drect->bottom += NIBLSCALE(12);
		break;
	default:
		break;

    }
}
#endif

