/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawInfoPage.cpp,v 1.1 2010/12/11 14:25:09 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"
#include "LKMapWindow.h"
#include "RGB.h"
#include "DoInits.h"
#include "ScreenGeometry.h"
#include "Asset.hpp"

#define AMBERCOLOR (IsDithered() ? RGB_WHITE : RGB_AMBER)


inline static
LKColor LineColor() {
	return (IsDithered()
		   ? (INVERTCOLORS ? RGB_WHITE : RGB_BLACK)
		   : RGB_DARKGREEN);
}

int InfoPageTopLineSeparator=0;

void VDrawLine(LKSurface& Surface, const RECT& rc, int x1, int y1, int x2, int y2, const LKColor& col) {
    const POINT p0({ x1, y1 });
    const POINT p1({ x2, y2 });
    Surface.DrawLine(PEN_SOLID, IsDithered() ? ScreenThinSize : NIBLSCALE(1), p0, p1, col, rc);
}

void MapWindow::DrawInfoPage(LKSurface& Surface,  const RECT& rc, bool forceinit )
{
  //SIZE TextSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];
  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR Empty[2]; // 100407
  int index=-1;

  static short	column[PANELCOLUMNS+1], hcolumn[(PANELCOLUMNS*2)+1], qcolumn[(PANELCOLUMNS*4)+1];
  static short	row[PANELROWS+1], hrow[(PANELCOLUMNS*2)+1], qrow[(PANELROWS*4)+1];

  bool showunit=false;
  _tcscpy(Empty,_T(""));

  if (forceinit) DoInit[MDI_DRAWINFOPAGE]=true;

	const auto oldfont = Surface.SelectObject(LKINFOFONT); // save font

  if (DoInit[MDI_DRAWINFOPAGE]) {
	DoInit[MDI_DRAWINFOPAGE]=false;
	// function can only be called in fullscreen  and thus can be inited here
	column[0]=rc.left+LEFTLIMITER;
	column[1]=((rc.right-RIGHTLIMITER-LEFTLIMITER-rc.left)/PANELCOLUMNS)+LEFTLIMITER+rc.left;
	column[2]=column[1]*2-LEFTLIMITER-rc.left;
	column[3]=column[1]*3-LEFTLIMITER*2-rc.left;
	column[PANELCOLUMNS]=rc.right-RIGHTLIMITER;
	row[0]=rc.top+TOPLIMITER;
	row[1]=((rc.bottom-BottomSize-row[0]-BOTTOMLIMITER)/PANELROWS)+row[0];
	row[2]=((rc.bottom-BottomSize-row[0]-BOTTOMLIMITER)/PANELROWS)*2+row[0];
	row[3]=((rc.bottom-BottomSize-row[0]-BOTTOMLIMITER)/PANELROWS)*3+row[0];
	row[PANELROWS]=rc.bottom-BottomSize-BOTTOMLIMITER;

	hcolumn[0]=column[0];
	hcolumn[1]=(column[1]-column[0])/2;
	hcolumn[2]=column[1];
	hcolumn[3]=(column[2]-column[1])/2+column[1];
	hcolumn[4]=column[2];
	hcolumn[5]=(column[3]-column[2])/2+column[2];
	hcolumn[6]=column[3];
	hcolumn[7]=(column[4]-column[3])/2+column[3];
	hcolumn[8]=column[4];

	hrow[0]=row[0];
	hrow[1]=(row[1]-row[0])/2+rc.top;
	hrow[2]=row[1];
	hrow[3]=(row[2]-row[1])/2+row[1];
	hrow[4]=row[2];
	hrow[5]=(row[3]-row[2])/2+row[2];
	hrow[6]=row[3];
	hrow[7]=(row[4]-row[3])/2+row[3];
	hrow[8]=row[4];

	qcolumn[0]=hcolumn[0];
	qcolumn[1]=(hcolumn[1]-hcolumn[0])/2+hcolumn[0];
	qcolumn[2]=hcolumn[1];
	qcolumn[3]=(hcolumn[2]-hcolumn[1])/2+hcolumn[1];
	qcolumn[4]=hcolumn[2];
	qcolumn[5]=(hcolumn[3]-hcolumn[2])/2+hcolumn[2];
	qcolumn[6]=hcolumn[3];
	qcolumn[7]=(hcolumn[4]-hcolumn[3])/2+hcolumn[3];
	qcolumn[8]=hcolumn[4];
	qcolumn[9]=(hcolumn[5]-hcolumn[4])/2+hcolumn[4];
	qcolumn[10]=hcolumn[5];
	qcolumn[11]=(hcolumn[6]-hcolumn[5])/2+hcolumn[5];
	qcolumn[12]=hcolumn[6];
	qcolumn[13]=(hcolumn[7]-hcolumn[6])/2+hcolumn[6];
	qcolumn[14]=hcolumn[7];
	qcolumn[15]=(hcolumn[8]-hcolumn[7])/2+hcolumn[7];
	qcolumn[16]=hcolumn[8];

	qrow[0]=hrow[0];
	qrow[1]=(hrow[1]-hrow[0])/2+hrow[0];
	qrow[2]=hrow[1];
	qrow[3]=(hrow[2]-hrow[1])/2+hrow[1];
	qrow[4]=hrow[2];
	qrow[5]=(hrow[3]-hrow[2])/2+hrow[2];
	qrow[6]=hrow[3];
	qrow[7]=(hrow[4]-hrow[3])/2+hrow[3];
	qrow[8]=hrow[4];
	qrow[9]=(hrow[5]-hrow[4])/2+hrow[4];
	qrow[10]=hrow[5];
	qrow[11]=(hrow[6]-hrow[5])/2+hrow[5];
	qrow[12]=hrow[6];
	qrow[13]=(hrow[7]-hrow[6])/2+hrow[6];
	qrow[14]=hrow[7];
	qrow[15]=(hrow[8]-hrow[7])/2+hrow[7];
	qrow[16]=hrow[8];

	qrow[5]+=NIBLSCALE(6);
	qrow[6]+=NIBLSCALE(6);
	qrow[7]+=NIBLSCALE(6);
	qrow[8]+=NIBLSCALE(6)*2;
	qrow[9]+=NIBLSCALE(6)*2;
	qrow[10]+=NIBLSCALE(6)*2;
	qrow[11]+=NIBLSCALE(6)*3;
	qrow[12]+=NIBLSCALE(6)*3;
	qrow[13]+=NIBLSCALE(6)*3;

        InfoPageTopLineSeparator=qrow[2];

  } // doinit

#include "./LKMW3include2.cpp"

	Surface.SelectObject(LK8PanelBigFont);
#include "./LKMW3include1.cpp"

	int curtype; // 100404
	bool ontarget=false;
	FLARM_TRAFFIC *pTarget=NULL;
	if ( (MapSpaceMode != MSM_INFO_TRF) && (MapSpaceMode != MSM_INFO_TARGET) ) {
		curtype=CURTYPE;
	} else {
		if (MapSpaceMode == MSM_INFO_TRF)
			curtype=IM_TRF + IM_TOP;
		else
			curtype=IM_TARGET + IM_TOP;

		if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) {
			ontarget=false;
		} else {
			if (DrawInfo.FLARM_Traffic[LKTargetIndex].ID <=0) {
				ontarget=false;
			} else {
				ontarget=true;
				pTarget=&DrawInfo.FLARM_Traffic[LKTargetIndex];
			}
		}
	}

	if (ontarget) DoTarget(&DrawInfo, &DerivedDrawInfo);

	switch (LKevent) {
		case LKEVENT_NONE:
			break;
		case LKEVENT_ENTER:
			break;
		case LKEVENT_NEWRUN:
			break;
		case LKEVENT_PAGEUP:
			break;
		case LKEVENT_PAGEDOWN:
			break;
		default:
			break;
	}
	LKevent=LKEVENT_NONE;

	LKColor icolor; // 091229

	// R0 C0	= status
	Surface.SelectObject(LK8PanelMediumFont);
	switch(curtype) {
		case IM_THERMAL:
			_stprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, MsgToken(905)); // Thermal
			break;
		case IM_CRUISE:
			_stprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, MsgToken(906)); // Cruise
			break;
		case IM_TASK:
			_stprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, MsgToken(907)); // Task
			break;
		case IM_AUX:
			_stprintf(Buffer,_T("%d.%d %s"),ModeIndex, curtype+1, MsgToken(908)); // Custom
			break;
		case IM_TRI:
#ifndef LKCOMPETITION
			_stprintf(Buffer,_T("%d.%d %s"), ModeIndex, curtype+1, MsgToken(909)); // Turn
#else
			_stprintf(Buffer,_T("%d.%d %s"), ModeIndex, curtype+1, MsgToken(1600)); // DISABLED
#endif
			break;
		case IM_HSI:
			_stprintf(Buffer,_T("%d.%d %s"), ModeIndex, curtype+1, MsgToken(1860)); // HSI
			break;
		case IM_CONTEST:
			  _stprintf(Buffer,_T("%d.%d %s"), ModeIndex, curtype+1, CContestMgr::XCRuleToString(AdditionalContestRule)); // Contest
			break;
		case IM_TRF+IM_TOP:
			_stprintf(Buffer,_T("%d.%d %s"), ModeIndex, IM_TRF+1, MsgToken(910)); // Target
			break;
		case IM_TARGET+IM_TOP:
			_stprintf(Buffer,_T("%d.%d %s"), ModeIndex, IM_TARGET+1, MsgToken(911)); // Sight
			break;
		default:
			_stprintf(Buffer,_T("error"));
			break;
	}
        LKWriteText(Surface, Buffer, qcolumn[0],qrow[0], WTMODE_NORMAL, WTALIGN_LEFT,IsDithered()?RGB_WHITE:RGB_LIGHTGREEN,false);

	// R0 C1
	icolor=RGB_WHITE;
	switch(curtype) {
		case IM_THERMAL:
		case IM_CRUISE:
		case IM_TASK:
		case IM_AUX:
			if ( ValidTaskPoint(ActiveTaskPoint) != false ) {
				index = Task[ActiveTaskPoint].Index;
				if ( index >=0 ) {
					_tcscpy(Buffer, WayPointList[index].Name);
				} else {
					_tcscpy(Buffer,MsgToken(912)); // [no dest]
					icolor=AMBERCOLOR;
				}
			} else {
				_tcscpy(Buffer,MsgToken(912)); // [no dest]
				icolor=AMBERCOLOR;
			}
			break;
		case IM_TRI:
#ifndef LKCOMPETITION
			_tcscpy(Buffer,MsgToken(913)); // Experimental
#else
			_tcscpy(Buffer,_T("---"));
#endif
			break;
		case IM_CONTEST:
		case IM_HSI: //for the HSI the title text is computed in his section down
			_tcscpy(Buffer,TEXT(""));
			break;
		case IM_TRF+IM_TOP:
		case IM_TARGET+IM_TOP:
			if (ontarget) {
				switch (pTarget->Status ) {
					case LKT_GHOST:
						icolor=RGB_LIGHTYELLOW;
						break;
					case LKT_ZOMBIE:
						icolor=RGB_LIGHTRED;
						break;
					default:
						icolor=RGB_WHITE;
						break;
				}
				//TCHAR status[80];
				if (_tcslen(pTarget->Name) == 1) {
					_stprintf(Buffer,_T("%0x"),(unsigned)pTarget->ID);
				} else {
					_stprintf(Buffer,_T("%s"),pTarget->Name);
				}

			} else {
				_tcscpy(Buffer,MsgToken(914)); // [no target]
				icolor=AMBERCOLOR;
			}

			break;
		default:
			_stprintf(Buffer,_T("error"));
			icolor=AMBERCOLOR;
			break;
	}
        LKWriteText(Surface, Buffer, qcolumn[8],qrow[1], WTMODE_NORMAL, WTALIGN_CENTER, icolor, false);

	// R0 C2	= time of day
	if ( (curtype == (IM_TOP+IM_TRF)) || (curtype == (IM_TOP+IM_TARGET)) ) {
		if (ontarget) {
			TCHAR tpas[30];
			Units::TimeToTextDown(tpas,(int)(DrawInfo.Time - pTarget->Time_Fix));

			switch (pTarget->Status) {
				case LKT_REAL:
					_stprintf(Buffer,_T("LIVE"));
					break;
				case LKT_GHOST:
					_stprintf(Buffer,_T("ghost %s\""),tpas);
					break;
				case LKT_ZOMBIE:
					_stprintf(Buffer,_T("zombie %s\""),tpas);
					break;
				default:
					_stprintf(Buffer,_T("??? %s\""),tpas);
					break;
			}
			LKWriteText(Surface, Buffer, qcolumn[16],qrow[0], WTMODE_NORMAL, WTALIGN_RIGHT, icolor, false);
		} else {
			LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle); // 091219
			LKWriteText(Surface, BufferValue, qcolumn[16],qrow[0], WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
		}
		if ( curtype == (IM_TOP+IM_TARGET) ) goto label_Target;
	} else {
		LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle); // 091219
		LKWriteText(Surface, BufferValue, qcolumn[16],qrow[0], WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	}

	if (curtype == IM_TRI) goto label_TRI;
	if (curtype == IM_HSI) goto label_HSI;



	VDrawLine(Surface, rc, qcolumn[0], qrow[2], qcolumn[16], qrow[2], LineColor());
	VDrawLine(Surface, rc, qcolumn[0], qrow[8], qcolumn[16], qrow[8], LineColor());

	// R1 C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(1);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CONTEST:
		    if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_FF_DIST, true, BufferValue, BufferUnit, BufferTitle);
		    else
			  showunit=LKFormatValue(LK_OLC_CLASSIC_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4], &qrow[3],&qrow[4],&qrow[2]);
			break;
	}

	// R1 C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_BRG, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_BRGDIFF, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_FT_DIST, true, BufferValue, BufferUnit, BufferTitle);
            else
			  showunit=LKFormatValue(LK_OLC_FAI_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(2);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_TO, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9],&qcolumn[8], &qrow[3],&qrow[4],&qrow[2]);
			break;
	}


	// R1 C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_FAI_DIST, true, BufferValue, BufferUnit, BufferTitle);
            else
			  showunit=LKFormatValue(LK_OLC_LEAGUE_DIST, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(3);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_GR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12], &qrow[3],&qrow[4],&qrow[2]);
			break;
	}

	// R1 C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16],
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16],
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_DIST, true, BufferValue, BufferUnit, BufferTitle);
            else
			  showunit=LKFormatValue(LK_OLC_3TPS_DIST, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16],
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_AUX:
			index=GetInfoboxType(4);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16],
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_ALTARRIV, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16],
											&qrow[3],&qrow[4],&qrow[2]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16],&qcolumn[16],
											&qrow[3],&qrow[4],&qrow[2]);
			break;
	}

	// R2  C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_FF_SCORE, true, BufferValue, BufferUnit, BufferTitle);
            else
			  showunit=LKFormatValue(LK_OLC_CLASSIC_PREDICTED_DIST, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(5);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4],&qcolumn[4], &qrow[6],&qrow[7],&qrow[5]);

	// R2  C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_BRG, true , BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_FT_SCORE, true, BufferValue, BufferUnit, BufferTitle);
            else
			  showunit=LKFormatValue(LK_OLC_FAI_PREDICTED_DIST, true , BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_AUX:
			index=GetInfoboxType(6);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_BEARING, true , BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8], &qrow[6],&qrow[7],&qrow[5]);
			break;
	}

	// R2  C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_VARIO, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_LD_CRUISE, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_FAI_SCORE, true, BufferValue, BufferUnit, BufferTitle);
            else {
              _tcscpy(BufferValue, _T(""));
              _tcscpy(BufferTitle, _T(""));
            }
			break;
		case IM_AUX:
			index=GetInfoboxType(7);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, true , BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[6],&qrow[7],&qrow[5]);

	// R2  C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_FL, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_LD_INST, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_SCORE, true, BufferValue, BufferUnit, BufferTitle);
            else
			  showunit=LKFormatValue(LK_OLC_3TPS_PREDICTED_DIST, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		case IM_AUX:
			index=GetInfoboxType(8);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);

	// R3  C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_GAIN, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) ) {
				showunit=LKFormatValue(LK_XC_MEAN_SPEED, false, BufferValue, BufferUnit, BufferTitle);
			} else {
				showunit = LKFormatValue(LK_OLC_CLASSIC_SPEED, false, BufferValue, BufferUnit, BufferTitle);
			}
			break;
		case IM_AUX:
			index=GetInfoboxType(9);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_ALT, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[9],&qrow[10],&qrow[8]);

	// R3  C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_DIST, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) ){
				_tcscpy(BufferValue, _T(""));
				_tcscpy(BufferTitle, _T(""));
			} else
			  showunit=LKFormatValue(LK_OLC_FAI_SPEED, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_AUX:
			index=GetInfoboxType(10);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_SPEED, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[8],&qrow[9],&qrow[10],&qrow[8]);
			break;
	}

	// R3  C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_AVG, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_TASK_DISTCOV, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) ){
              _tcscpy(BufferValue, _T(""));
              _tcscpy(BufferTitle, _T(""));
            } else
              showunit=LKFormatValue(LK_OLC_LEAGUE_SPEED, true, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_AUX:
			index=GetInfoboxType(11);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_VARIO, false, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[9],&qrow[10],&qrow[8]);


	// R3  C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_TC_ALL, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_FL, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_GR, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) ){
              _tcscpy(BufferValue, _T(""));
              _tcscpy(BufferTitle, _T(""));
            } else
              showunit=LKFormatValue(LK_OLC_3TPS_SPEED, true, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		case IM_AUX:
			index=GetInfoboxType(12);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_AVGVARIO, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);


	// R4  C1
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_WIND, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[5], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_WIND, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[5], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_FIN_ALTDIFF0, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CONTEST:
            if ( static_cast<int>(AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC)) ){
              showunit=LKFormatValue(LK_XC_PREDICTED_DIST, true, BufferValue, BufferUnit, BufferTitle);
			} else
              showunit=LKFormatValue(LK_OLC_PLUS_SCORE, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_AUX:
			index=GetInfoboxType(13);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TRF+IM_TOP:
			LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			break;
	}

	// R4  C2
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_TL_AVG, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_LKFIN_ETE, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) ){
              showunit=LKFormatValue(LK_XC_CLOSURE_PERC, true, BufferValue, BufferUnit, BufferTitle);
			} else
              showunit=LKFormatValue(LK_OLC_PLUS_PREDICTED_SCORE, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_AUX:
			index=GetInfoboxType(14);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_TARGET_EIAS, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[8], &qcolumn[8],&qrow[12],&qrow[13],&qrow[11]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[9], &qcolumn[9],&qrow[12],&qrow[13],&qrow[11]);
			break;
	}

	// R4  C3
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CRUISE:
			showunit=LKFormatValue(LK_TC_ALL, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TASK:
			showunit=LKFormatValue(LK_SPEEDTASK_ACH, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) )
              showunit=LKFormatValue(LK_XC_CLOSURE_DIST, true, BufferValue, BufferUnit, BufferTitle);
            else
              showunit=LKFormatValue(LK_OLC_LEAGUE_SCORE, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_AUX:
			index=GetInfoboxType(15);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[12], &qcolumn[12],&qrow[12],&qrow[13],&qrow[11]);
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[13], &qcolumn[13],&qrow[12],&qrow[13],&qrow[11]);
			break;
	}

	// R4  C4
	showunit=false;
	switch(curtype) {
		case IM_THERMAL:
			showunit=LKFormatValue(LK_EMPTY, false, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CRUISE:
		case IM_TASK:
			showunit=LKFormatValue(LK_MC, true, BufferValue, BufferUnit, BufferTitle);
			break;
		case IM_CONTEST:
            if ( AdditionalContestRule!=static_cast<int>(CContestMgr::ContestRule::OLC) ){
              _tcscpy(BufferValue, _T(""));
              _tcscpy(BufferTitle, _T(""));
            } else {
              _tcscpy(BufferValue, _T(""));
              _tcscpy(BufferTitle, _T(""));
            }
			break;
		case IM_AUX:
			index=GetInfoboxType(16);
			showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
		case IM_TRF+IM_TOP:
			showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
			break;
		default:
			LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			break;
	}
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
	goto label_End;

	// This is the TRI page
	//
	// ATTENTION PLEASE
	// Some values are using direct access to CALCULATED_INFO for fast responses.
	// Other values are using the copy of struct made 1 second later
	//
label_TRI:
#ifndef LKCOMPETITION
	VDrawLine(Surface,rc, qcolumn[0],qrow[2],qcolumn[16],qrow[2],LineColor());
	DrawTRI(Surface, rc);
	showunit=true; // 091219
	if (ScreenLandscape) {
		// right
		LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[3],&qrow[4],&qrow[2]);
		LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[6],&qrow[7],&qrow[5]);
		LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[15], &qcolumn[15],&qrow[9],&qrow[10],&qrow[8]);
		LKFormatValue(LK_VARIO, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[12],&qrow[13],&qrow[11]);

		// left
		LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);
		LKFormatValue(LK_BANK_ANGLE, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[3],&qrow[4],&qrow[2]);
		LKFormatValue(LK_GLOAD, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);
		LKFormatValue(LK_ODOMETER, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
	} else {
		// right
		LKFormatValue(LK_TRACK, true, BufferValue, BufferUnit, BufferTitle);
		_tcscpy(BufferUnit,_T(""));
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[14], &qcolumn[14],&qrow[3],&qrow[4],&qrow[2]);
		LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		_tcscpy(BufferUnit,_T(""));
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);
		LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle);
		_tcscpy(BufferUnit,_T(""));
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);
		LKFormatValue(LK_VARIO, true, BufferValue, BufferUnit, BufferTitle);
		_tcscpy(BufferUnit,_T(""));
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);

		// left
		LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
		_tcscpy(BufferUnit,_T(""));
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);
		LKFormatValue(LK_BANK_ANGLE, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[3],&qrow[4],&qrow[2]);
		LKFormatValue(LK_GLOAD, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);
		LKFormatValue(LK_ODOMETER, true, BufferValue, BufferUnit, BufferTitle);
		WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
	}
#if 0
	_stprintf(BufferValue,_T("%0.1f"),CALCULATED_INFO.TurnRate);
	_tcscpy(BufferTitle,_T("Rate"));
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);

	LKFormatValue(LK_GLOAD, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
#endif
	
	// NOT FOR IFR USAGE
	Surface.SelectObject(LK8PanelSmallFont);
	LKWriteText(Surface, MsgToken(915), qcolumn[8],qrow[12], WTMODE_OUTLINED, WTALIGN_CENTER, RGB_ORANGE, false);

#endif // not in LKCOMPETITION
	goto label_End; // End of TRI

	// This is the HSI page
label_HSI:
	VDrawLine(Surface,rc, qcolumn[0],qrow[2],qcolumn[16],qrow[2],LineColor());

	static bool showQFU=false, showVFRlanding=false;
	{
		bool usingQFU=false, approach=false, landing=false;
		DrawHSI(Surface,rc,usingQFU,approach,landing);
		if(landing) {
			showVFRlanding=!showVFRlanding; //make it blinking
			if(usingQFU) showQFU=!showVFRlanding;
			else showQFU=false;
		} else {
			showVFRlanding=false;
			if(usingQFU) showQFU=!showQFU; //make it blinking
			else showQFU=false;
		}
		if(showVFRlanding || showQFU) { //show QFU or "VFR landing"
			if(showVFRlanding) {
				_stprintf(Buffer,TEXT("VFR %s"),MsgToken(931)); //TODO: toupper()
				if (!IsDithered()) {
					icolor = INVERTCOLORS ? RGB_YELLOW : RGB_DARKYELLOW;
				} else {
					icolor = RGB_WHITE;
				}
			}
			if(showQFU) {
				_stprintf(Buffer, TEXT("QFU: %d%s"),WayPointList[Task[ActiveTaskPoint].Index].RunwayDir,MsgToken(2179));
				icolor = IsDithered() ? RGB_WHITE : RGB_GREEN;
			}
		} else { //show next waypoint name
			icolor=RGB_WHITE;
			if(ValidTaskPoint(ActiveTaskPoint)) {
				if(Task[ActiveTaskPoint].Index >=0) _tcscpy(Buffer, WayPointList[Task[ActiveTaskPoint].Index].Name);
				else {
					_tcscpy(Buffer,MsgToken(912)); // [no dest]
					icolor=AMBERCOLOR;
				}
			} else {
				_tcscpy(Buffer,MsgToken(912)); // [no dest]
				icolor=AMBERCOLOR;
			}
		}
		Surface.SelectObject(LK8PanelMediumFont);
		LKWriteText(Surface,  Buffer, qcolumn[8],qrow[1], WTMODE_NORMAL, WTALIGN_CENTER, icolor, false);
		showunit=true;
		if (ScreenLandscape) {
			LKFormatValue(LK_NEXT_ETE, true, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[3],&qrow[4],&qrow[2]);
			LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[7],&qrow[8],&qrow[6]);
			LKFormatValue(LK_NEXT_ETA, true, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			if(!approach) { //if not landing print also dist, ETE and ETA respect task end
				LKFormatValue(LK_FIN_ETE, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[3],&qrow[4],&qrow[2]);
				LKFormatValue(LK_FIN_DIST, true, BufferValue, BufferUnit, BufferTitle);
				if(ScreenSize==ss800x480 || ScreenSize==ss480x272)
					WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[15], &qcolumn[15],&qrow[7],&qrow[8],&qrow[6]);
				else {
					_tcscpy(BufferUnit,_T(""));
					WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[7],&qrow[8],&qrow[6]);
				}
				LKFormatValue(LK_FIN_ETA, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
			} else { //show other interesting things for landing
				int col=16;
				bool unitInvisible=true;
				if(ScreenSize==ss800x480 || ScreenSize==ss480x272) {
					col=15;
					unitInvisible=false;
				}
				LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
				if(unitInvisible) _tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[col], &qcolumn[col],&qrow[3],&qrow[4],&qrow[2]);
				LKFormatValue(LK_HAGL, true, BufferValue, BufferUnit, BufferTitle);
				if(unitInvisible) _tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[col], &qcolumn[col],&qrow[12],&qrow[13],&qrow[11]);
			}
		} else {
			LKFormatValue(LK_NEXT_ETE, true, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[3],&qrow[4],&qrow[2]);
			LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);
			LKFormatValue(LK_NEXT_ETA, true, BufferValue, BufferUnit, BufferTitle);
			_tcscpy(BufferUnit,_T(""));
			WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[4], &qcolumn[4],&qrow[12],&qrow[13],&qrow[11]);
			if(!approach) { //if not landing print also dist, ETE and ETA respect task end
				LKFormatValue(LK_FIN_ETE, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[3],&qrow[4],&qrow[2]);
				LKFormatValue(LK_FIN_DIST, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);
				LKFormatValue(LK_FIN_ETA, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
			} else {
				LKFormatValue(LK_IAS, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[3],&qrow[4],&qrow[2]);
				LKFormatValue(LK_HAGL, true, BufferValue, BufferUnit, BufferTitle);
				_tcscpy(BufferUnit,_T(""));
				WriteInfo(Surface, &showunit, BufferValue, BufferUnit, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);
			}
		}
	}
	goto label_End; // End of HSI

	// Traffic Target page

label_Target:
	VDrawLine(Surface,rc, qcolumn[0],qrow[2],qcolumn[16],qrow[2],LineColor());
	// pass the sight rectangle to use on the screen. Warning: DrawTarget will cache these values
	// and will not use them after the first run time anymore...
	DrawTarget(Surface, rc, qrow[3],qrow[15],qcolumn[3],qcolumn[13]);

	showunit=false; // 091219

	// left

	showunit=LKFormatValue(LK_TARGET_DIST, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[3],&qrow[4],&qrow[2]);

	showunit=LKFormatValue(LK_TARGET_EIAS, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[6],&qrow[7],&qrow[5]);

	showunit=LKFormatValue(LK_TARGET_AVGVARIO, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[9],&qrow[10],&qrow[8]);

	showunit=LKFormatValue(LK_TARGET_GR, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[3], &qcolumn[3],&qrow[12],&qrow[13],&qrow[11]);

	// right
	showunit=LKFormatValue(LK_TARGET_ALTDIFF, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[3],&qrow[4],&qrow[2]);

	showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[6],&qrow[7],&qrow[5]);

	showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[9],&qrow[10],&qrow[8]);

	showunit=LKFormatValue(LK_TARGET_ALTARRIV, true, BufferValue, BufferUnit, BufferTitle);
	WriteInfo(Surface, &showunit, BufferValue, Empty, BufferTitle, &qcolumn[16], &qcolumn[16],&qrow[12],&qrow[13],&qrow[11]);


label_End:
  // restore font and return
  Surface.SelectObject(oldfont);

}



void MapWindow::WriteInfo(LKSurface& Surface, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle,
				short *columnvalue, short *columntitle, short *row1, short *row2, short *row3) {

  static short unitrowoffset=0;
  if (DoInit[MDI_WRITEINFO]) {
	switch(ScreenSize) {
#if 1 // TODO WE SHOULD USE GENERAL default, after CHECKING
		case ss800x480:
			unitrowoffset=10;
			break;
		case ss640x480:
			unitrowoffset=5;
			break;
		case ss400x240:
			unitrowoffset=7;
			break;
		case ss480x272:
			unitrowoffset=5;
			break;
		case ss480x234:
			unitrowoffset=3;
			break;
		case ss320x240:
			unitrowoffset=3;
			break;
		// portrait mode
		case ss240x320:
			unitrowoffset=-5;
			break;
		case ss272x480:
			unitrowoffset=-14;
			break;
		case ss480x640:
			unitrowoffset=-8;
			break;
		case ss480x800:
			unitrowoffset=-19;
			break;
#endif
		default:
			switch(ScreenGeometry) {
			    case SCREEN_GEOMETRY_43:
				if (ScreenLandscape)
			            unitrowoffset=(int)(5.0*Screen0Ratio);
				else
			            unitrowoffset=(int)(-8.0*Screen0Ratio);
			        break;
			    case SCREEN_GEOMETRY_53:
				if (ScreenLandscape)
			            unitrowoffset=(int)(10.0*Screen0Ratio);
				else
			            unitrowoffset=(int)(-19.0*Screen0Ratio);
			        break;
			    case SCREEN_GEOMETRY_169:
				if (ScreenLandscape)
			            unitrowoffset=(int)(5.0*Screen0Ratio);
				else
			            unitrowoffset=(int)(-14.0*Screen0Ratio);
			        break;
			    default:
			        break;
			}
			break;
	}
	DoInit[MDI_WRITEINFO]=false;
  }

  Surface.SelectObject(LK8PanelBigFont);
  if (*showunit)
	LKWriteText(Surface, BufferValue, *columnvalue,*row1, WTMODE_NORMAL,WTALIGN_RIGHT, RGB_WHITE, false);
  else
	LKWriteText(Surface, BufferValue, *columnvalue,*row1, WTMODE_NORMAL,WTALIGN_RIGHT, AMBERCOLOR, false);

  if (*showunit==true && !HideUnits) {
	Surface.SelectObject(LK8PanelUnitFont); // 091230
        LKWriteText(Surface, BufferUnit, *columnvalue,*row2+unitrowoffset, WTMODE_NORMAL, WTALIGN_LEFT, RGB_WHITE, false);
  }
  Surface.SelectObject(LK8PanelSmallFont);
	if (!IsDithered()) {
		LKWriteText(Surface, BufferTitle, *columntitle, *row3, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_LIGHTGREEN, false);
	} else {
		LKWriteText(Surface, BufferTitle, *columntitle, *row3, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_WHITE, false);
	}

}
