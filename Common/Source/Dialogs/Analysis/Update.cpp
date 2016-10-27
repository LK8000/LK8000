/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "McReady.h"
#include "Atmosphere.h"
#include "ContestMgr.h"

extern CContestMgr::TType contestType;

extern int analysis_page;

void UpdateAnalysis(WndForm* pForm){

  WndOwnerDrawFrame *waGrid = (WndOwnerDrawFrame*)pForm->FindByName(TEXT("frmGrid"));
  LKASSERT(waGrid);
  if (!waGrid) return;

  WndOwnerDrawFrame *waInfo = (WndOwnerDrawFrame*)pForm->FindByName(TEXT("frmInfo"));
  LKASSERT(waInfo);
  if (!waInfo) return;


  TCHAR sTmp[1000];

  TCHAR szPolarName[80];
  extern void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext);
  LK_tsplitpath(szPolarFile, (TCHAR*) NULL, (TCHAR*) NULL, szPolarName, (TCHAR*) NULL);

  switch(analysis_page){
    case ANALYSIS_PAGE_BAROGRAPH:
      _stprintf(sTmp, TEXT("%s: %s"),
	// LKTOKEN  _@M93_ = "Analysis"
                MsgToken(93),
	// LKTOKEN  _@M127_ = "Barograph"
                MsgToken(127));
      pForm->SetCaption(sTmp);
      if (flightstats.Altitude_Ceiling.sum_n<2) {
        _tcscpy(sTmp, TEXT("\0"));
      } else if (flightstats.Altitude_Ceiling.sum_n<4) {
        _stprintf(sTmp, TEXT("%s:\r\n  %.0f-%.0f %s"),
	// LKTOKEN  _@M823_ = "Working band"
                  MsgToken(823),
                  flightstats.Altitude_Base.y_ave*ALTITUDEMODIFY,
                  flightstats.Altitude_Ceiling.y_ave*ALTITUDEMODIFY,
                  Units::GetAltitudeName());

      } else {

        _stprintf(sTmp, TEXT("%s:\r\n  %.0f-%.0f %s\r\n\r\n%s:\r\n  %.0f %s/hr"),
	// LKTOKEN  _@M823_ = "Working band"
                  MsgToken(823),
                  flightstats.Altitude_Base.y_ave*ALTITUDEMODIFY,
                  flightstats.Altitude_Ceiling.y_ave*ALTITUDEMODIFY,
                  Units::GetAltitudeName(),
	// LKTOKEN  _@M165_ = "Ceiling trend"
                  MsgToken(165),
                  flightstats.Altitude_Ceiling.m*ALTITUDEMODIFY,
                  Units::GetAltitudeName());
      }
      waInfo->SetCaption(sTmp);

    break;
    case ANALYSIS_PAGE_CLIMB:
      _stprintf(sTmp, TEXT("%s: %s"),
	// LKTOKEN  _@M93_ = "Analysis"
                MsgToken(93),
	// LKTOKEN  _@M182_ = "Climb"
                MsgToken(182));
      pForm->SetCaption(sTmp);

      if (flightstats.ThermalAverage.sum_n==0) {
        _tcscpy(sTmp, TEXT("\0"));
      } else if (flightstats.ThermalAverage.sum_n==1) {
        _stprintf(sTmp, TEXT("%s:\r\n  %3.1f %s"),
	// LKTOKEN  _@M116_ = "Av climb"
                  MsgToken(116),
                  flightstats.ThermalAverage.y_ave*LIFTMODIFY,
                  Units::GetVerticalSpeedName()
                  );
      } else {
        _stprintf(sTmp, TEXT("%s:\r\n  %3.1f %s\r\n\r\n%s:\r\n  %3.2f %s"),
	// LKTOKEN  _@M116_ = "Av climb"
                  MsgToken(116),
                  flightstats.ThermalAverage.y_ave*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
	// LKTOKEN  _@M181_ = "Climb trend"
                  MsgToken(181),
                  flightstats.ThermalAverage.m*LIFTMODIFY,
                  Units::GetVerticalSpeedName()
                  );
      }

      waInfo->SetCaption(sTmp);

    break;
    case ANALYSIS_PAGE_WIND:
      _stprintf(sTmp, TEXT("%s: %s"),
	// LKTOKEN  _@M93_ = "Analysis"
                MsgToken(93),
	// LKTOKEN  _@M820_ = "Wind at Altitude"
                MsgToken(820));
      pForm->SetCaption(sTmp);
      _stprintf(sTmp, TEXT(" "));
      waInfo->SetCaption(sTmp);
    break;
    case ANALYSIS_PAGE_POLAR:
	if (ScreenLandscape) {
		_stprintf(sTmp, TEXT("%s: %s %s (%s %3.0f kg)"),
	                MsgToken(93), // Analysis:
	                szPolarName,
	                MsgToken(325),  // Glide Polar
	                MsgToken(889), // Mass
	                GlidePolar::GetAUW());
	} else {
		// Portrait reduced size
		_stprintf(sTmp, TEXT("%s: %s (%3.0f kg)"),
	                MsgToken(93), // Analysis:
	                szPolarName,
	                GlidePolar::GetAUW());

	}
      pForm->SetCaption(sTmp);
      _stprintf(sTmp, TEXT(" "));
      waInfo->SetCaption(sTmp);
    break;
  case ANALYSIS_PAGE_TEMPTRACE:
    _stprintf(sTmp, TEXT("%s: %s"),
	// LKTOKEN  _@M93_ = "Analysis"
              MsgToken(93),
	// LKTOKEN  _@M701_ = "Temp trace"
              MsgToken(701));
    pForm->SetCaption(sTmp);

    _stprintf(sTmp, TEXT("%s:\r\n  %5.0f %s\r\n\r\n%s:\r\n  %5.0f %s\r\n"),
	// LKTOKEN  _@M714_ = "Thermal height"
	      MsgToken(714),
	      CuSonde::thermalHeight*ALTITUDEMODIFY,
	      Units::GetAltitudeName(),
	// LKTOKEN  _@M187_ = "Cloud base"
	      MsgToken(187),
	      CuSonde::cloudBase*ALTITUDEMODIFY,
	      Units::GetAltitudeName());

    waInfo->SetCaption(sTmp);
    break;
  case ANALYSIS_PAGE_TASK_SPEED:
    _stprintf(sTmp, TEXT("%s: %s"),
	// LKTOKEN  _@M93_ = "Analysis"
              MsgToken(93),
	// LKTOKEN  _@M697_ = "Task speed"
              MsgToken(697));
    pForm->SetCaption(sTmp);
    waInfo->SetCaption(TEXT(""));
    break;
  case ANALYSIS_PAGE_TASK:
      TCHAR FAI[10];
      if(CALCULATED_INFO.TaskFAI)
	_stprintf(FAI, TEXT("FAI"));
      else
	_tcscpy(FAI, TEXT(""));

    _stprintf(sTmp, TEXT("%s: %s %.0f%s %s"),
	// LKTOKEN  _@M93_ = "Analysis"
              MsgToken(93),
	// LKTOKEN  _@M699_ = "Task"
              MsgToken(699),
		  DISTANCEMODIFY*CALCULATED_INFO.TaskTotalDistance,
		  Units::GetDistanceName(),
		  FAI);
    pForm->SetCaption(sTmp);

    RefreshTaskStatistics();

    if (!ValidTaskPoint(ActiveTaskPoint)) {
	// LKTOKEN  _@M476_ = "No task"
      _tcscpy(sTmp, MsgToken(476));
    } else
   {
      TCHAR timetext1[100];
      TCHAR timetext2[100];
      if (AATEnabled) {
        Units::TimeToText(timetext1, (int)CALCULATED_INFO.TaskTimeToGo);
        Units::TimeToText(timetext2, (int)CALCULATED_INFO.AATTimeToGo);

        if (ScreenLandscape) {
          _stprintf(sTmp,
                    TEXT("%s:\r\n  %s\r\n%s:\r\n  %s\r\n%s:\r\n  %5.0f %s\r\n%s%.0f %s\r\n"), // 100429
	// LKTOKEN  _@M698_ = "Task to go"
                    MsgToken(698),
                    timetext1,
	// LKTOKEN  _@M42_ = "AAT to go"
                    MsgToken(42),
                    timetext2,
	// LKTOKEN  _@M242_ = "Dist to go"
                    MsgToken(242),
                    DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance,
                    Units::GetDistanceName(),
	// LKTOKEN  _@M626_ = "Sp "
                    MsgToken(626),
                    TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed,
                    Units::GetTaskSpeedName()
                    );
        } else {
          _stprintf(sTmp,
                    TEXT("%s: %s\r\n%s: %s\r\n%s: %5.0f %s\r\n%s: %5.0f %s \r\n"),
	// LKTOKEN  _@M698_ = "Task to go"
                    MsgToken(698),
                    timetext1,
	// LKTOKEN  _@M42_ = "AAT to go"
                    MsgToken(42),
                    timetext2,
	// LKTOKEN  _@M242_ = "Dist to go"
                    MsgToken(242),
                    DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance,
                    Units::GetDistanceName(),
	// LKTOKEN  _@M681_ = "Targ.speed"
                    MsgToken(681),
                    TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed,
                    Units::GetTaskSpeedName()
                    );
        }
      } else {
        Units::TimeToText(timetext1, (int)CALCULATED_INFO.TaskTimeToGo);
        _stprintf(sTmp, TEXT("%.0f%s %s\r\n\r\n%s: %s\r\n%s: %.0f%s\r\n"),
			  DISTANCEMODIFY*CALCULATED_INFO.TaskTotalDistance,
			  Units::GetDistanceName(),
			  FAI ,
	// LKTOKEN  _@M698_ = "Task to go"
                  MsgToken(698),
                  timetext1,
	// LKTOKEN  _@M242_ = "Dist to go"
                  MsgToken(242),
                  DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo,
                  Units::GetDistanceName()
        );
      }
    }
    waInfo->SetCaption(sTmp);
    break;
  case ANALYSIS_PAGE_CONTEST:
    _stprintf(sTmp, TEXT("%s: %s - %s"),
              // LKTOKEN  _@M93_ = "Analysis"
              MsgToken(93),
              // LKTOKEN  _@M1450_ = "Contest"
              MsgToken(1450),
              CContestMgr::TypeToString(contestType));
    pForm->SetCaption(sTmp);

    {
      bool typeFAITriangle = false;

      CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, false);
      switch (contestType) /* temporary change optimizer mode for analyzer calculations */
      {
        case CContestMgr::TYPE_FAI_TRIANGLE:  typeFAITriangle = true; FAI_OptimizerMode =3; break;
        case CContestMgr::TYPE_FAI_TRIANGLE4: typeFAITriangle = true; FAI_OptimizerMode =4; break;
#ifdef  FIVEPOINT_OPTIMIZER
        case CContestMgr::TYPE_FAI_TRIANGLE5: typeFAITriangle = true; FAI_OptimizerMode =5;	break;
#endif
        default: break;
      }

      if(typeFAITriangle)
	  {
		 result = CContestMgr::Instance().Result(CContestMgr::TYPE_FAI_TRIANGLE, false);
		 CContestMgr::Instance().RefreshFAIOptimizer();
	  }

      if ((result.Type() == contestType) || typeFAITriangle)
      {
        BOOL bFAI = CContestMgr::Instance().FAI();
        double  fDist     = result.Distance();
        if(!bFAI && ( typeFAITriangle))  // was only !bFAI
		fDist /=2.0;
        double  fCPDist   = CContestMgr::Instance().GetClosingPointDist();
        double  fB_CPDist = CContestMgr::Instance().GetBestClosingPointDist();
    // 	LKASSERT( fDist >0 );
        if(fDist < 10.0)
          fDist= 1000.0;


        TCHAR distStr[50];  TCHAR speedStr[50];
        if(typeFAITriangle && bFAI)
          _stprintf(distStr, _T("%.1f %s FAI\r\n"), DISTANCEMODIFY * fDist, Units::GetDistanceName());
        else
          _stprintf(distStr, _T("%.1f %s\r\n"), DISTANCEMODIFY * fDist, Units::GetDistanceName());

        if(typeFAITriangle && bFAI )
          _stprintf(speedStr, _T("%s%-.1f %s\r\n(%.1f %%)\r\n"),MsgToken(1508),  DISTANCEMODIFY * fCPDist, Units::GetDistanceName(),fCPDist/fDist*100.0);
        else
          _stprintf(speedStr, TEXT("%.1f %s\r\n"),TASKSPEEDMODIFY * result.Speed(), Units::GetTaskSpeedName());
        TCHAR timeTempStr[50];
        Units::TimeToText(timeTempStr, result.Duration());
        TCHAR timeStr[50];

        if( typeFAITriangle && bFAI && ISPARAGLIDER)
          _stprintf(timeStr, _T("\r\n%s%-.1f %s\r\n(%.1f %%)\r\n"),MsgToken(1511), DISTANCEMODIFY * fB_CPDist, Units::GetDistanceName(), fB_CPDist/fDist*100.0); //_@M1511_ = "B:"
        else
          _stprintf(timeStr, _T("%s\r\n"), timeTempStr);

        TCHAR scoreStr[50] = _T("");
        if(result.Type() != CContestMgr::TYPE_FAI_3_TPS &&
           result.Type() != CContestMgr::TYPE_FAI_TRIANGLE &&
           result.Type() != CContestMgr::TYPE_FAI_TRIANGLE4 &&
#ifdef  FIVEPOINT_OPTIMIZER
           result.Type() != CContestMgr::TYPE_FAI_TRIANGLE5 &&
#endif
           result.Type() != CContestMgr::TYPE_FAI_3_TPS_PREDICTED)
          _stprintf(scoreStr, TEXT("%.2f pt\r\n"), result.Score());

        TCHAR plusStr[50] = _T("");
        if(result.Type() == CContestMgr::TYPE_OLC_CLASSIC ||
           result.Type() == CContestMgr::TYPE_OLC_CLASSIC_PREDICTED ||
           result.Type() == CContestMgr::TYPE_OLC_FAI ||
           result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) {
          CContestMgr::TType type = (result.Type() == CContestMgr::TYPE_OLC_CLASSIC_PREDICTED ||
                                     result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) ?
            CContestMgr::TYPE_OLC_PLUS_PREDICTED : CContestMgr::TYPE_OLC_PLUS;
          CContestMgr::CResult resultPlus = CContestMgr::Instance().Result(type, false);
          if(ScreenLandscape)
            _stprintf(plusStr, TEXT("\r\n%s:\r\n%.2f pt"),
                      CContestMgr::TypeToString(type),
                      resultPlus.Score());
          else
            _stprintf(plusStr, TEXT("\r\n%s: %.2f pt"),
                      CContestMgr::TypeToString(type),
                      resultPlus.Score());
        }

        _stprintf(sTmp, _T("%s%s%s%s%s"), distStr, speedStr, timeStr, scoreStr, plusStr);
      }
      else {
        _stprintf(sTmp, TEXT("%s\r\n"),
                  // LKTOKEN  _@M477_ = "No valid path"
                  MsgToken(477));
      }
      waInfo->SetCaption(sTmp);
    }

    break;

  }

  if (waGrid != NULL)
    waGrid->Redraw();

}
