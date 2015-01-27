/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "Dialogs.h"
#include "DoInits.h"


extern bool ValidAirspace(int i);

void MapWindow::DrawAspNearest(LKSurface& Surface, const RECT& rc) {


  SIZE ASPTextSize, DSTextSize, BETextSize, TYTextSize, ACTextSize, HLTextSize, MITextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  static RECT s_sortBox[6]; 
  static TCHAR Buffer1[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][30], Buffer2[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][10];
  static TCHAR Buffer3[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][10];
  static TCHAR Buffer4[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][12], Buffer5[MAXNEARAIRSPACES][MAXAIRSPACENUMPAGES][12];
  static short s_maxnlname;
  static TCHAR s_trailspace[3];
  short i, k, iRaw, wlen, rli=0, curpage, drawn_items_onpage;
  double value;
  LKColor rcolor;

  // column0 starts after writing 1:2 (ModeIndex:CURTYPE+1) with a different font..
  static short Column0;
  static short Column1, Column2, Column3, Column4, Column5;
  static POINT p1, p2;
  static short s_rawspace;
  // Printable area for live nearest values
  static short left,right,bottom;
  // one for each mapspace, no matter if 0 and 1 are unused

  // we lock to current mapspace for this drawing
  short curmapspace=MapSpaceMode; 

  static int AspNumraws=0;


  // Vertical and horizontal spaces
  #define INTERRAW	1
  #define HEADRAW	NIBLSCALE(6)	
  BrushReference sortbrush;
  RECT invsel;

  if (INVERTCOLORS) {
  	sortbrush=LKBrush_LightGreen;
  } else {
  	sortbrush=LKBrush_DarkGreen;
  }

  if (DoInit[MDI_DRAWASPNEAREST]) {

  // Set screen borders to avoid writing on extreme pixels
  if ( !ScreenLandscape ) {
	// Portrait mode has tight horizontal margins...
	left=rc.left+NIBLSCALE(1);
	right=rc.right-NIBLSCALE(1);
  	bottom=rc.bottom-BottomSize-NIBLSCALE(2);
	s_maxnlname=7; 
  	_stprintf(Buffer,TEXT("AKSJSMMMM"));  
  } else {
	left=rc.left+NIBLSCALE(5);
	right=rc.right-NIBLSCALE(5);
  	bottom=rc.bottom-BottomSize;
	s_maxnlname=15; 
  	_stprintf(Buffer,TEXT("ABCDEF GHIJK-LM"));  
	// now resize for tuning on resolutions
	if (ScreenSize == ss320x240) s_maxnlname=9;
	if (ScreenSize == ss400x240) s_maxnlname=10;
  }
  Buffer[s_maxnlname]='\0';


  /// WPT is now AIRSPACE name
  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest  was LK8Title
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &ASPTextSize);

  // DST is always distance
  _stprintf(Buffer,TEXT("000.0")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &DSTextSize);

  // Bearing
  _stprintf(Buffer,TEXT("<<123")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &BETextSize);

  // TYPE, 4 letters printed
  #define LKASP_TYPE_LEN	4
  _stprintf(Buffer,TEXT("CTRA")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &TYTextSize);

  // Flags can be SFE, three chars
  _stprintf(Buffer,TEXT("SFE")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &ACTextSize);

  Surface.SelectObject(LK8InfoNormalFont);
  _stprintf(Buffer,TEXT("MMMM")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &HLTextSize);

  Surface.SelectObject(LK8PanelMediumFont);
  _stprintf(Buffer,TEXT("1.1")); 
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &MITextSize);

  short afterwpname=left+ASPTextSize.cx+NIBLSCALE(5);
  short intercolumn=(right-afterwpname- DSTextSize.cx-BETextSize.cx-TYTextSize.cx-ACTextSize.cx)/3; 

  // Col0 is where ASP 1/3 can be written, after ModeIndex:Curtype
  Column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
  Column1=left;							// WP align left
  Column2=afterwpname+TYTextSize.cx;				// TY align right
  Column3=Column2+intercolumn+DSTextSize.cx;			// DS align right
  Column4=Column3+intercolumn+BETextSize.cx;			// BE align right
  Column5=Column4+intercolumn+ACTextSize.cx;			// AC align right


  if ( !ScreenLandscape ) {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW;
  	AspNumraws=(bottom - TopSize) / (ASPTextSize.cy+(INTERRAW*2));
  	if (AspNumraws>MAXNEARAIRSPACES) AspNumraws=MAXNEARAIRSPACES;
  	s_rawspace=(ASPTextSize.cy+INTERRAW);
	Column5=rc.right-NIBLSCALE(1)-1;
	_tcscpy(s_trailspace,_T(""));
  } else {
  	TopSize=rc.top+HEADRAW*2+HLTextSize.cy;
  	p1.x=0; p1.y=TopSize; p2.x=rc.right; p2.y=p1.y;
  	TopSize+=HEADRAW/2;
  	AspNumraws=(bottom - TopSize) / (ASPTextSize.cy+INTERRAW);
  	if (AspNumraws>MAXNEARAIRSPACES) AspNumraws=MAXNEARAIRSPACES;
  	s_rawspace=(ASPTextSize.cy+INTERRAW);
	_tcscpy(s_trailspace,_T(" "));
  }

#define INTERBOX intercolumn/2

  s_sortBox[0].left=Column0; 
  if ( !ScreenLandscape ) s_sortBox[0].right=left+ASPTextSize.cx-NIBLSCALE(2);
  else s_sortBox[0].right=left+ASPTextSize.cx-NIBLSCALE(10);
  s_sortBox[0].top=2;
  s_sortBox[0].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][0]=s_sortBox[0].right;

  if ( !ScreenLandscape ) s_sortBox[1].left=Column1+afterwpname-INTERBOX;
  else s_sortBox[1].left=Column1+afterwpname-INTERBOX-NIBLSCALE(2);
  s_sortBox[1].right=Column2+INTERBOX;
  s_sortBox[1].top=2;
  s_sortBox[1].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][1]=s_sortBox[1].right;

  s_sortBox[2].left=Column2+INTERBOX;
  s_sortBox[2].right=Column3+INTERBOX;
  s_sortBox[2].top=2;
  s_sortBox[2].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][2]=s_sortBox[2].right;

  s_sortBox[3].left=Column3+INTERBOX;
  s_sortBox[3].right=Column4+INTERBOX;
  s_sortBox[3].top=2;
  s_sortBox[3].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][3]=s_sortBox[3].right;

  s_sortBox[4].left=Column4+INTERBOX;
  s_sortBox[4].right=rc.right-1;
  s_sortBox[4].top=2;
  s_sortBox[4].bottom=p1.y;
  SortBoxX[MSM_AIRSPACES][4]=s_sortBox[4].right;

  SortBoxY[MSM_AIRSPACES]=p1.y;

  AspNumpages=roundupdivision(MAXNEARAIRSPACES, AspNumraws);
  if (AspNumpages>MAXAIRSPACENUMPAGES) AspNumpages=MAXAIRSPACENUMPAGES;
  else if (AspNumpages<1) AspNumpages=1;

  SelectedRaw[MSM_AIRSPACES]=0;
  SelectedPage[MSM_AIRSPACES]=0;

  DoInit[MDI_DRAWASPNEAREST]=false;
  return;
  } // doinit

  bool ndr;
  ndr=DoAirspaces(&DrawInfo, &DerivedDrawInfo);

  AspNumpages=roundupdivision(LKNumAirspaces, AspNumraws);
  if (AspNumpages>MAXAIRSPACENUMPAGES) AspNumpages=MAXAIRSPACENUMPAGES;
  else if (AspNumpages<1) AspNumpages=1;

  curpage=SelectedPage[curmapspace];
  if (curpage<0||curpage>=MAXAIRSPACENUMPAGES) { // TODO also >Numpages
	SelectedPage[curmapspace]=0;
	LKevent=LKEVENT_NONE;
	return;
  }

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		LKevent=LKEVENT_NONE;
		i=LKSortedAirspaces[SelectedRaw[curmapspace] + (curpage*AspNumraws)];

		if ( ValidAirspace(i)) {
            CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
            CAirspaceManager::Instance().PopupAirspaceDetail(LKAirspaces[i].Pointer);
        }
		LKevent=LKEVENT_NONE; 
		return;
		break;
	case LKEVENT_DOWN:
		if (++SelectedRaw[curmapspace] >=AspNumraws) SelectedRaw[curmapspace]=0;
		break;
	case LKEVENT_UP:
		if (--SelectedRaw[curmapspace] <0) SelectedRaw[curmapspace]=AspNumraws-1;
		break;
	case LKEVENT_PAGEUP:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_PAGEDOWN:
		LKevent=LKEVENT_NONE;
		break;
	case LKEVENT_NEWRUN:
		for (i=0; i<MAXNEARAIRSPACES; i++) {
			for (k=0; k<MAXAIRSPACENUMPAGES; k++) {
				_stprintf(Buffer1[i][k], _T("----------------------------"));  // max 30
				Buffer1[i][k][s_maxnlname+7]='\0'; // some more dashes 
				_stprintf(Buffer2[i][k],_T("----"));
				_stprintf(Buffer3[i][k],_T("----"));
				_stprintf(Buffer4[i][k],_T("----"));
				_stprintf(Buffer5[i][k],_T("  "));
			}
		}
		break;
	case LKEVENT_NEWPAGE:
		break;
	default:
		LKevent=LKEVENT_NONE;
		break;
  }

  if (INVERTCOLORS)
	  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, rc);
  else
	  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_DARKGREEN, rc);

  Surface.SelectObject(LK8InfoNormalFont); // Heading line

  short cursortbox=SortedMode[curmapspace];

  if ( !ScreenLandscape ) { // portrait mode
	Surface.FillRect(&s_sortBox[cursortbox], sortbrush);

	_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  	Surface.SelectObject(LK8PanelMediumFont);

	LKWriteText(Surface, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  	Surface.SelectObject(LK8InfoNormalFont);

	_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1642_")), curpage+1, AspNumpages);  // ASP

	if (cursortbox == 0)
		LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

	_tcscpy(Buffer,gettext(TEXT("_@M752_"))); // Type
	if (cursortbox==1)
		LKWriteText(Surface, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column2, HEADRAW , 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_tcscpy(Buffer,gettext(TEXT("_@M1300_")));  // Dist
	if (cursortbox==2)
		LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	_tcscpy(Buffer,gettext(TEXT("_@M1301_")));  // Dir
	if (cursortbox==3)
		LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

	// Active mode
	_tcscpy(Buffer,TEXT("*")); 
	if (cursortbox==4)
		LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
	else
		LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);


  } else {
	Surface.FillRect(&s_sortBox[cursortbox], sortbrush);

		_stprintf(Buffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  		Surface.SelectObject(LK8PanelMediumFont);
		LKWriteText(Surface, Buffer, LEFTLIMITER, rc.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);
  		Surface.SelectObject(LK8InfoNormalFont);

		_stprintf(Buffer,TEXT("%s %d/%d"), gettext(TEXT("_@M1642_")), curpage+1, AspNumpages);  // ASP

		if (cursortbox==0)
			LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, false);
		else
			LKWriteText(Surface, Buffer, Column0, HEADRAW-NIBLSCALE(1) , 0,WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

		_tcscpy(Buffer,gettext(TEXT("_@M752_"))); // Type
		if (cursortbox==1)
			LKWriteText(Surface, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface, Buffer, Column2, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_tcscpy(Buffer,gettext(TEXT("_@M1300_")));  // Dist
		if (cursortbox==2)
			LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface, Buffer, Column3, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_tcscpy(Buffer,gettext(TEXT("_@M1301_")));  // dir
		if (cursortbox==3)
			LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface, Buffer, Column4, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

		_stprintf(Buffer,TEXT("*"));
		if (cursortbox==4)
			LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_BLACK, false);
		else
			LKWriteText(Surface, Buffer, Column5, HEADRAW , 0,WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);

  } // landscape mode


  Surface.SelectObject(LK8InfoBigFont); // Text font for Nearest


  int *psortedindex;
  psortedindex=LKSortedAirspaces;

  for (i=0, drawn_items_onpage=0; i<AspNumraws; i++) {
	iRaw=TopSize+(s_rawspace*i);
	short curraw=(curpage*AspNumraws)+i;
	if (curraw>=MAXNEARAIRSPACES) break;

	rli=*(psortedindex+curraw);

	if (!ndr) {
		goto KeepOldValues;
	}
	if ( ValidAirspace(rli) ) {

		//
		// AIRSPACE NAME
		//
		wlen=_tcslen(LKAirspaces[rli].Name);
		if (wlen>s_maxnlname) {
			LK_tcsncpy(Buffer, LKAirspaces[rli].Name, s_maxnlname);
		}
		else {
			LK_tcsncpy(Buffer, LKAirspaces[rli].Name, wlen);
		}
		CharUpper(Buffer);
		_tcscpy(Buffer1[i][curpage],Buffer); 


		//
		// AIRSPACE TYPE
		//
		wlen=_tcslen(LKAirspaces[rli].Type);
		if (wlen>LKASP_TYPE_LEN) {
			LK_tcsncpy(Buffer, LKAirspaces[rli].Type, LKASP_TYPE_LEN);
		}
		else {
			LK_tcsncpy(Buffer, LKAirspaces[rli].Type, wlen);
		}
		CharUpper(Buffer);
		_tcscpy(Buffer2[i][curpage],Buffer); 

		
		//
		// AIRSPACE DISTANCE
		//
		switch(LKAirspaces[rli].WarningLevel) {
			case awYellow:
				value=LKAirspaces[rli].Distance*DISTANCEMODIFY;
       				_stprintf(Buffer3[i][curpage],TEXT("%0.1lf!"),value);
				break;
			case awRed:
       				_stprintf(Buffer3[i][curpage],TEXT("IN"));
				break;
			default:
				value=LKAirspaces[rli].Distance*DISTANCEMODIFY;
       				_stprintf(Buffer3[i][curpage],TEXT("%0.1lf"),value);
				break;
		}

		//
		// AIRSPACE BEARING DIFFERENCE, OR BEARING IF CIRCLING
		//
		if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
			value = LKAirspaces[rli].Bearing -  DrawInfo.TrackBearing;

			if (value < -180.0)
				value += 360.0;
			else
				if (value > 180.0)
					value -= 360.0;

			if (value > 1)
				_stprintf(Buffer4[i][curpage], TEXT("%2.0f%s%s"), value, gettext(_T("_@M2179_")), gettext(_T("_@M2183_")));
			else if (value < -1)
				_stprintf(Buffer4[i][curpage], TEXT("%s%2.0f%s"), gettext(_T("_@M2182_")), -value, gettext(_T("_@M2179_")));
            else
			    _stprintf(Buffer4[i][curpage], TEXT("%s%s"), gettext(_T("_@M2182_")), gettext(_T("_@M2183_")));
		} else
			_stprintf(Buffer4[i][curpage], TEXT("%2.0f%s"), LKAirspaces[rli].Bearing, gettext(_T("_@M2179_")));


		//
		// AIRSPACE FLAGS
		//
		TCHAR aspflags[5];
		_stprintf(aspflags,_T("%s%s%s%s"),
			LKAirspaces[rli].Selected ? _T("S") : _T(""),
			LKAirspaces[rli].Flyzone  ? _T("F") : _T("  "),
			LKAirspaces[rli].Enabled  ? _T("E") : _T("D"), s_trailspace);

		_stprintf(Buffer5[i][curpage], TEXT("%s"), aspflags);

	} else {
		_stprintf(Buffer1[i][curpage], _T("----------------------------"));  // max 30
		Buffer1[i][curpage][s_maxnlname+7]='\0'; // some more dashes 
		_stprintf(Buffer2[i][curpage],_T("----"));
		_stprintf(Buffer3[i][curpage],_T("----"));
		_stprintf(Buffer4[i][curpage],_T("----"));
		_stprintf(Buffer5[i][curpage],_T("  "));
	}


KeepOldValues:

	if ( ValidAirspace(rli) ) {

		drawn_items_onpage++;

		switch(LKAirspaces[rli].WarningLevel) {
			case awYellow:
				rcolor=RGB_LIGHTYELLOW;
  				Surface.SelectObject(LK8InfoBigItalicFont);
				break;
			case awRed:
				rcolor=RGB_LIGHTRED;
  				Surface.SelectObject(LK8InfoBigItalicFont);
				break;
			case awNone:
			default:
				rcolor=RGB_WHITE;
  				Surface.SelectObject(LK8InfoBigFont);
				break;
		}
	} else {
		rcolor=RGB_GREY;
	}

	LKWriteText(Surface, Buffer1[i][curpage], Column1, iRaw , 0, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
	
	LKWriteText(Surface, Buffer2[i][curpage], Column2, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface, Buffer3[i][curpage], Column3, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface, Buffer4[i][curpage], Column4, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

	LKWriteText(Surface, Buffer5[i][curpage], Column5, iRaw , 0, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);

  } 


  if (LKevent==LKEVENT_NEWRUN || LKevent==LKEVENT_NEWPAGE ) {
		LKevent=LKEVENT_NONE;
		return;
  }

  if (drawn_items_onpage>0) {

	if (SelectedRaw[curmapspace] <0 || SelectedRaw[curmapspace]>(AspNumraws-1)) {
		return;
	}
	if (SelectedRaw[curmapspace] >= drawn_items_onpage) {
		if (LKevent==LKEVENT_DOWN) SelectedRaw[curmapspace]=0;
		else 
		if (LKevent==LKEVENT_UP) SelectedRaw[curmapspace]=drawn_items_onpage-1;
		else {
			// DoStatusMessage(_T("Cant find valid raw")); not needed anymore
			SelectedRaw[curmapspace]=0;
		}
	}

	
	invsel.left=left;
	invsel.right=right;
	invsel.top=TopSize+(s_rawspace*SelectedRaw[curmapspace])+NIBLSCALE(2);
	invsel.bottom=TopSize+(s_rawspace*(SelectedRaw[curmapspace]+1))-NIBLSCALE(1);
	Surface.InvertRect(invsel);

  } 

  LKevent=LKEVENT_NONE;
  return;
}

// True if the i airspace is existing and valid
bool ValidAirspace(int i) {
  if (i<0 || i>MAXNEARAIRSPACES) return false;
  return LKAirspaces[i].Valid;
}
