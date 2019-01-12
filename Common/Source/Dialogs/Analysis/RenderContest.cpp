/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ContestMgr.h"
#include "LKObjects.h"
#include "NavFunctions.h"
#include "DrawFAIOpti.h"
CContestMgr::TType contestType = CContestMgr::TYPE_OLC_CLASSIC;

/*****************************************************************
 * Alpha Lima splitted RenderContest from Render Task for CC
 * adding FAI Sector display
 ****************************************************************/
void Statistics::RenderContest(LKSurface &Surface, const RECT &rc) {

  if (contestType == CContestMgr::TYPE_FAI_ASSISTANT) {
    RenderFAIOptimizer(Surface, rc);
  } else {
    double fXY_Scale = 1.0;


    double lat1 = 0;
    double lon1 = 0;
    double lat2 = 0;
    double lon2 = 0;
    double x1, y1, x2 = 0, y2 = 0;
    double lat_c, lon_c;

    ResetScale();


    CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, true);
    const CPointGPSArray &points = result.PointArray();


    // find center

    CPointGPSArray trace;
    CContestMgr::Instance().Trace(trace);
    for (unsigned ui = 0; ui < trace.size(); ui++) {
      lat1 = trace[ui].Latitude();
      lon1 = trace[ui].Longitude();
      ScaleYFromValue(rc, lat1);
      ScaleXFromValue(rc, lon1);
    }

    const auto hfOldU = Surface.SelectObject(LK8PanelUnitFont);
    lat_c = (y_max + y_min) / 2;
    lon_c = (x_max + x_min) / 2;


    // find scale
    ResetScale();

    lat1 = GPS_INFO.Latitude;
    lon1 = GPS_INFO.Longitude;
    x1 = (lon1 - lon_c) * fastcosine(lat1);
    y1 = (lat1 - lat_c);
    ScaleXFromValue(rc, x1 * fXY_Scale);
    ScaleYFromValue(rc, y1 * fXY_Scale);
    for (unsigned ui = 0; ui < trace.size(); ui++) {
      lat1 = trace[ui].Latitude();
      lon1 = trace[ui].Longitude();
      x1 = (lon1 - lon_c) * fastcosine(lat1);
      y1 = (lat1 - lat_c);
      ScaleXFromValue(rc, x1 * fXY_Scale);
      ScaleYFromValue(rc, y1 * fXY_Scale);
    }

    ScaleMakeSquare(rc);



    // draw track
    if (!trace.empty()) {
      for (unsigned ui = 0; trace.size() && ui < trace.size() - 1; ui++) {
        lat1 = trace[ui].Latitude();
        lon1 = trace[ui].Longitude();
        lat2 = trace[ui + 1].Latitude();
        lon2 = trace[ui + 1].Longitude();
        x1 = (lon1 - lon_c) * fastcosine(lat1);
        y1 = (lat1 - lat_c);
        x2 = (lon2 - lon_c) * fastcosine(lat2);
        y2 = (lat2 - lat_c);
        DrawLine(Surface, rc, x1, y1, x2, y2, STYLE_MEDIUMBLACK);
      }
    }
    // Draw aircraft on top
    double lat_p = GPS_INFO.Latitude;
    double lon_p = GPS_INFO.Longitude;
    double xp = (lon_p - lon_c) * fastcosine(lat_p);
    double yp = (lat_p - lat_c);


    if (result.Type() == contestType && !points.empty()) {
      for (unsigned ui = 0; ui < points.size() - 1; ui++) {
        lat1 = points[ui].Latitude();
        lon1 = points[ui].Longitude();
        lat2 = points[ui + 1].Latitude();
        lon2 = points[ui + 1].Longitude();

        x1 = (lon1 - lon_c) * fastcosine(lat1);
        y1 = (lat1 - lat_c);
        x2 = (lon2 - lon_c) * fastcosine(lat2);
        y2 = (lat2 - lat_c);
        int style = STYLE_REDTHICK;
        if ((result.Type() == CContestMgr::TYPE_OLC_FAI ||
            result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) &&
            (ui == 0 || ui == 3)) {
          // triangle start and finish
          style = STYLE_DASHGREEN;
        } else if (result.Predicted() &&
            (result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED ||
                result.Type() == CContestMgr::TYPE_XC_FAI_TRIANGLE ||
                result.Type() == CContestMgr::TYPE_XC_FREE_TRIANGLE ||
                result.Type() == CContestMgr::TYPE_FAI_ASSISTANT ||
                ui == points.size() - 2)) {
          // predicted edge
          style = STYLE_BLUETHIN;
        }

        if ((result.Type() == CContestMgr::TYPE_FAI_3_TPS) ||// TYPE_FAI_3_TPS_PREDICTED
            (result.Type() == CContestMgr::TYPE_FAI_3_TPS_PREDICTED)) {
        }

        if ((contestType != CContestMgr::TYPE_FAI_ASSISTANT))
          DrawLine(Surface, rc, x1, y1, x2, y2, style);
      }


      if (result.Type() == CContestMgr::TYPE_OLC_FAI ||
          result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED ||
          result.Type() == CContestMgr::TYPE_XC_FREE_TRIANGLE ||
          result.Type() == CContestMgr::TYPE_XC_FAI_TRIANGLE ||
          result.Type() == CContestMgr::TYPE_FAI_ASSISTANT) {
        // draw the last edge of a triangle
        lat1 = points[1].Latitude();
        lon1 = points[1].Longitude();
        lat2 = points[3].Latitude();
        lon2 = points[3].Longitude();
        x1 = (lon1 - lon_c) * fastcosine(lat1);
        y1 = (lat1 - lat_c);
        x2 = (lon2 - lon_c) * fastcosine(lat2);
        y2 = (lat2 - lat_c);

        DrawLine(Surface, rc, x1, y1, x2, y2, result.Predicted() ? STYLE_BLUETHIN : STYLE_REDTHICK);

      }

      DrawXGrid(Surface, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
      DrawYGrid(Surface, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);

      Surface.SelectObject(hfOldU);
#ifndef DITHER
      Surface.SetTextColor(RGB_MAGENTA);
#else
      Surface.SetTextColor(RGB_BLACK);
#endif
      Surface.SetBackgroundTransparent();
      DrawLabel(Surface, rc, TEXT("O"), xp, yp);
    }
  }
}


/*****************************************************************
 * Alpha Lima splitted RenderContest from Render Task for CC
 * adding FAI Sector display
 ****************************************************************/
void Statistics::RenderFAIOptimizer(LKSurface &Surface, const RECT &rc) {

  unsigned int ui;
  double fXY_Scale = 2.6;
  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  double x1, y1, x2 = 0, y2 = 0;
  double lat_c, lon_c;
  double lat_p = GPS_INFO.Latitude;
  double lon_p = GPS_INFO.Longitude;
#define DRAWPERCENT
#ifdef DRAWPERCENT
  double fTotalPercent = 1.0;
#endif

  ResetScale();
  static FAI_Sector ContestFAISector[4];


  CContestMgr::CResult result = CContestMgr::Instance().Result(CContestMgr::TYPE_FAI_ASSISTANT, true);
  fXY_Scale = 2.5;

  // find center
  double fTotalDistance = result.Distance();
  if (fTotalDistance < 1) fTotalDistance = 1;
  CPointGPSArray trace;
  CContestMgr::Instance().Trace(trace);
  for (ui = 0; ui < trace.size(); ui++) {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    ScaleYFromValue(rc, lat1);
    ScaleXFromValue(rc, lon1);
  }


  const auto hfOldU = Surface.SelectObject(LK8PanelUnitFont);
  BOOL bFAITri = CContestMgr::Instance().FAI();
  double fDist, fAngle;
  lat_c = (y_max + y_min) / 2;
  lon_c = (x_max + x_min) / 2;

  double xp = (lon_p - lon_c) * fastcosine(lat_p);
  double yp = (lat_p - lat_c);

  // find scale
  ResetScale();

  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1 - lon_c) * fastcosine(lat1);
  y1 = (lat1 - lat_c);
  ScaleXFromValue(rc, x1 * fXY_Scale);
  ScaleYFromValue(rc, y1 * fXY_Scale);
  for (ui = 0; ui < trace.size(); ui++) {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    x1 = (lon1 - lon_c) * fastcosine(lat1);
    y1 = (lat1 - lat_c);
    ScaleXFromValue(rc, x1 * fXY_Scale);
    ScaleYFromValue(rc, y1 * fXY_Scale);
  }

  ScaleMakeSquare(rc);


  // Draw FAI sectors
  const CContestMgr::TriangleLeg *max_leg = CContestMgr::Instance().GetFAIAssistantMaxLeg();
  if ( max_leg != nullptr && max_leg->LegDist >= FAI_MIN_DISTANCE_THRESHOLD) {

    const CContestMgr::TriangleLeg *leg0 = CContestMgr::Instance().GetFAIAssistantLeg(0);
    const CContestMgr::TriangleLeg *leg1 = CContestMgr::Instance().GetFAIAssistantLeg(1);
    const CContestMgr::TriangleLeg *leg2 = CContestMgr::Instance().GetFAIAssistantLeg(2);
    const double distance = leg0->LegDist + leg1->LegDist + leg2->LegDist;

    double fTic;
    if (!CContestMgr::Instance().LooksLikeAFAITriangleAttempt()) {
      // Does not look like a FAI attempt. Just draw both FAI sectors on longest leg.
      fTic = 10 / DISTANCEMODIFY;
      if (max_leg->LegDist > 5 / DISTANCEMODIFY) fTic = 20 / DISTANCEMODIFY;
      if (max_leg->LegDist > 50 / DISTANCEMODIFY) fTic = 50 / DISTANCEMODIFY;
      if (max_leg->LegDist > 100 / DISTANCEMODIFY) fTic = 100 / DISTANCEMODIFY;
      //  if(fDist_c > 200/DISTANCEMODIFY) fTic = 100/DISTANCEMODIFY;
      if (max_leg->LegDist > 500 / DISTANCEMODIFY) fTic = 250 / DISTANCEMODIFY;
      ContestFAISector[0].CalcSectorCache(max_leg->Lat1, max_leg->Lon1, max_leg->Lat2, max_leg->Lon2, fTic, 0);
      ContestFAISector[0].AnalysisDrawFAISector(Surface, rc, GeoPoint(lat_c, lon_c), RGB_LIGHTYELLOW);
      ContestFAISector[1].CalcSectorCache(max_leg->Lat1, max_leg->Lon1, max_leg->Lat2, max_leg->Lon2, fTic, 1);
      ContestFAISector[1].AnalysisDrawFAISector(Surface, rc, GeoPoint(lat_c, lon_c), RGB_LIGHTYELLOW);
    } else {
      if (CContestMgr::Instance().GetFAIAssistantLeg(0)->LegDist > FAI_MIN_DISTANCE_THRESHOLD) {
        fTic = 10 / DISTANCEMODIFY;
        if (leg0->LegDist > 5 / DISTANCEMODIFY) fTic = 20 / DISTANCEMODIFY;
        if (leg0->LegDist > 50 / DISTANCEMODIFY) fTic = 50 / DISTANCEMODIFY;
        if (leg0->LegDist > 100 / DISTANCEMODIFY) fTic = 100 / DISTANCEMODIFY;
        // Draw the yellow sector on the best current direction.
        ContestFAISector[1].CalcSectorCache(leg0->Lat1, leg0->Lon1, leg0->Lat2, leg0->Lon2, fTic, CContestMgr::Instance().isFAITriangleClockwise());
        ContestFAISector[1].AnalysisDrawFAISector(Surface, rc, GeoPoint(lat_c, lon_c), RGB_YELLOW);
      }
      // If a valid second leg (or a leg that belong to the current best FAI triangle ) draw it in the correct direction
      if (leg1->LegDist > distance * 0.25) {
        fTic = 10 / DISTANCEMODIFY;
        if (leg1->LegDist > 5 / DISTANCEMODIFY) fTic = 20 / DISTANCEMODIFY;
        if (leg1->LegDist > 50 / DISTANCEMODIFY) fTic = 50 / DISTANCEMODIFY;
        if (leg1->LegDist > 100 / DISTANCEMODIFY) fTic = 100 / DISTANCEMODIFY;
        ContestFAISector[3].CalcSectorCache(leg1->Lat1, leg1->Lon1, leg1->Lat2, leg1->Lon2, fTic, CContestMgr::Instance().isFAITriangleClockwise());
        ContestFAISector[3].AnalysisDrawFAISector(Surface, rc, GeoPoint(lat_c, lon_c), RGB_CYAN);
        if (leg2->LegDist > distance * 0.25) {
          fTic = 10 / DISTANCEMODIFY;
          if (leg2->LegDist > 5 / DISTANCEMODIFY) fTic = 20 / DISTANCEMODIFY;
          if (leg2->LegDist > 50 / DISTANCEMODIFY) fTic = 50 / DISTANCEMODIFY;
          if (leg2->LegDist > 100 / DISTANCEMODIFY) fTic = 100 / DISTANCEMODIFY;
          ContestFAISector[3].CalcSectorCache(leg2->Lat1, leg2->Lon1, leg2->Lat2, leg2->Lon2, fTic, CContestMgr::Instance().isFAITriangleClockwise());
          ContestFAISector[3].AnalysisDrawFAISector(Surface, rc, GeoPoint(lat_c, lon_c), RGB_GREEN);
        }
      }

    }
    // draw triangle
    for (ui = 0; ui < 3; ui++) {
      lat1 = CContestMgr::Instance().GetFAIAssistantLeg(ui)->Lat1;
      lon1 = CContestMgr::Instance().GetFAIAssistantLeg(ui)->Lon1;
      lat2 = CContestMgr::Instance().GetFAIAssistantLeg(ui)->Lat2;
      lon2 = CContestMgr::Instance().GetFAIAssistantLeg(ui)->Lon2;
      x1 = (lon1 - lon_c) * fastcosine(lat1);
      y1 = (lat1 - lat_c);
      x2 = (lon2 - lon_c) * fastcosine(lat2);
      y2 = (lat2 - lat_c);

      int style = STYLE_BLUETHIN;
      DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
#ifdef DRAWPERCENT
      if ((result.Distance() > 5000) && bFAITri) {
        TCHAR text[180];
        SIZE tsize;
        fTotalPercent -= fDist / result.Distance();
        _stprintf(text, TEXT("%3.1f%%"), (fDist / result.Distance() * 100.0));
        Surface.GetTextSize(text, &tsize);
#ifndef DITHER
        Surface.SetTextColor(RGB_BLUE);
#else
        Surface.SetTextColor(RGB_BLACK);
#endif
        Surface.DrawText(ScaleX(rc, x1 + (x2 - x1) / 2) - tsize.cx / 2, ScaleY(rc, y1 + (y2 - y1) / 2), text);
      }
#endif
      DrawLine(Surface, rc, x1, y1, x2, y2, style);
    }

    // Draw closing segment
    if (CContestMgr::Instance().FAI()) {
      lat1 = CContestMgr::Instance().GetFreeTriangleClosingPoint().Latitude();
      lon1 = CContestMgr::Instance().GetFreeTriangleClosingPoint().Longitude();
      x1 = (lon1 - lon_c) * fastcosine(lat1);
      y1 = (lat1 - lat_c);
      DrawLine(Surface, rc, x1, y1, xp, yp, STYLE_REDTHICK);
    }


  }



  // draw track
  for (ui = 0; trace.size() && ui < trace.size() - 1; ui++) {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    lat2 = trace[ui + 1].Latitude();
    lon2 = trace[ui + 1].Longitude();
    x1 = (lon1 - lon_c) * fastcosine(lat1);
    y1 = (lat1 - lat_c);
    x2 = (lon2 - lon_c) * fastcosine(lat2);
    y2 = (lat2 - lat_c);
    DrawLine(Surface, rc, x1, y1, x2, y2, STYLE_MEDIUMBLACK);
  }




  DrawXGrid(Surface, rc,
            1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
  DrawYGrid(Surface, rc,
            1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
  Surface.
      SetBackgroundTransparent();
#ifndef DITHER
  Surface.
      SetTextColor(RGB_MAGENTA
  );
#else
  Surface.SetTextColor(RGB_BLACK);
#endif
  DrawLabel(Surface, rc,
            TEXT("O"), xp, yp);
  Surface.
      SelectObject(hfOldU);

}
