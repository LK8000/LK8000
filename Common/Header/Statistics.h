/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Statistics.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef STATISTICS_H
#define STATISTICS_H

#include "leastsqs.h"
#include "Screen/LKSurface.h"
#define BORDER_X 15
#define BORDER_Y  19


class Statistics {
 public:

  enum {
    STYLE_BLUETHIN,
    STYLE_REDTHICK,
    STYLE_ORANGETHICK,
    STYLE_ORANGETHIN,
    STYLE_GREENTHICK,
    STYLE_GREENMEDIUM,
    STYLE_DASHGREEN,
    STYLE_MEDIUMBLACK,
    STYLE_THINDASHPAPER,
    STYLE_WHITETHICK
  };

  LeastSquares ThermalAverage;
  LeastSquares Wind_x;
  LeastSquares Wind_y;
  LeastSquares Altitude;
  LeastSquares Altitude_Base;
  LeastSquares Altitude_Ceiling;
  LeastSquares Task_Speed;
  double LegStartTime[MAXTASKPOINTS];
  LeastSquares Altitude_Terrain;


  void Reset();
  static void DrawBarChart(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata);

  static void DrawFilledLineGraph(LKSurface& Surface, const RECT& rc,
				  LeastSquares* lsdata,
				  const LKColor& thecolor);

  static void DrawLineGraph(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata,
                            int Style);
  static void DrawTrend(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata,
                        int Style);
  static void DrawTrendN(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata,
                         int Style);

  static void DrawLine(LKSurface& Surface, const RECT& rc,
		       double xmin,
		       double ymin,
                       double xmax,
		       double ymax, int Style);

  static int ScaleX(const RECT& rc, double x);

  static int ScaleY(const RECT& rc,double y);

  static void ScaleYFromData(const RECT& rc, LeastSquares* lsdata);
  static void ScaleXFromData(const RECT& rc, LeastSquares* lsdata);
  static void ScaleYFromValue(const RECT& rc, double val);
  static void ScaleXFromValue(const RECT& rc, double val);
  static void ScaleMakeSquare(const RECT& rc);

  static void StyleLine(LKSurface& Surface, const POINT& l1, const POINT& l2, int Style, const RECT& rc);



  static double yscale;
  static double xscale;
  static double y_min, x_min;
  static double x_max, y_max;
  static bool unscaled_x;
  static bool unscaled_y;
  static void ResetScale();

  static void FormatTicText(TCHAR *text, double val, double step);
  static void DrawXGrid(LKSurface& Surface, const RECT& rc,
                        double tic_step, double zero, int Style,
                        double unit_step, bool draw_units=false);
  static void DrawYGrid(LKSurface& Surface, const RECT& rc,
                        double tic_step, double zero, int Style,
                        double unit_step, bool draw_units=false);
  static void DrawYGrid_cor(LKSurface& Surface, const RECT& rc,
			double tic_step,
			double zero,
                        int Style,
			double unit_step, bool draw_units=false);
  static void DrawXLabel(LKSurface& Surface, const RECT& rc, const TCHAR *text);
  static void DrawYLabel(LKSurface& Surface, const RECT& rc, const TCHAR *text);
  static void DrawLabel(LKSurface& Surface, const RECT& rc, const TCHAR *text, double xv, double yv);
  static void DrawNoData(LKSurface& Surface, const RECT& rc);

  ///


    static void RenderBarograph(LKSurface& Surface, const RECT& rc);
    static void RenderClimb(LKSurface& Surface, const RECT& rc);
    static void RenderGlidePolar(LKSurface& Surface, const RECT& rc);
    static void RenderWind(LKSurface& Surface, const RECT& rc);
    static void RenderTemperature(LKSurface& Surface, const RECT& rc);
    static void RenderTask(LKSurface& Surface, const RECT& rc, bool olcmode);
    static void RenderContest(LKSurface& Surface, const RECT& rc);
    static void RenderFAIOptimizer(LKSurface& Surface, const RECT& rc);
    static void RenderSpeed(LKSurface& Surface, const RECT& rc);
};

#endif
