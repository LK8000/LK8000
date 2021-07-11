/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"

void MapWindow::DrawThermalBand(LKSurface& Surface, const RECT& rc)
{
  ThermalBarDrawn=false;
  if (!IsThermalBarVisible()) {
    return;
  }

  POINT GliderBand[6] = { {0,0},{23,0},{22,0},{24,0},{0,0}, {0,0} };

  #if 0  
  // THIS IS CONFUSING TO PEOPLE. AND IN CASE WE USE ThermalBar 3 (always) it should be 
  // in any case disabled.
  if ((DerivedDrawInfo.TaskAltitudeDifference>50)
      &&(mode.Is(Mode::MODE_FINAL_GLIDE))) {
    return;
  }
  #endif

  // JMW TODO accuracy: gather proper statistics
  // note these should/may also be relative to ground
  const double mth = DerivedDrawInfo.MaxThermalHeight;
  // no thermalling has been done above safety altitude
  if (mth<=1) {
    return;
  }
  
  double Wt[NUMTHERMALBUCKETS];
  double ht[NUMTHERMALBUCKETS];
  const int TBSCALEY = ( (rc.bottom - rc.top )/2)-NIBLSCALE(30);
#define TBSCALEX 20
  
  // calculate height above safety altitude
  const double hoffset = DerivedDrawInfo.TerrainBase;
  const double h = DerivedDrawInfo.NavAltitude-hoffset;

  const bool draw_start_height = ((ActiveTaskPoint==0) && (ValidTaskPoint(0)) 
			    && (StartMaxHeight!=0)
			    && (DerivedDrawInfo.TerrainValid));
  double hstart=0;
  if (draw_start_height) {
    if (StartHeightRef == 0) {
      hstart = (StartMaxHeight/1000)+DerivedDrawInfo.TerrainAlt; //@ 100315
    } else {
      hstart = StartMaxHeight/1000; // 100315
    }
    hstart -= hoffset;
  }

  const short lkvariooffset = rc.left + LKVarioBar?(LKVarioSize+1):0; 

  // calculate top/bottom height
  double maxh = max(h, mth);
  double minh = min(h, 0.0);

  if (draw_start_height) {
    maxh = max(maxh, hstart);
    minh = min(minh, hstart);
  }
  
  if (maxh < minh) {
    return;
  }

  // normalised heights
  LKASSERT((maxh-minh)!=0);
  const double hglider = (h-minh)/(maxh-minh);
  hstart = (hstart-minh)/(maxh-minh);

  // calculate averages
  unsigned numtherm = 0;

  const double mc = MACCREADY;
  double Wmax = max(0.5,mc);

  for (unsigned i=0; i<NUMTHERMALBUCKETS; ++i) {
    double wthis = 0;
    // height of this thermal point [0,mth]
    double hi = i*mth/NUMTHERMALBUCKETS;
    double hp = ((hi-minh)/(maxh-minh));

    if (DerivedDrawInfo.ThermalProfileN[i]>5) {
      // now requires 10 items in bucket before displaying,
      // to eliminate kinks
      wthis = DerivedDrawInfo.ThermalProfileW[i]
                 /DerivedDrawInfo.ThermalProfileN[i];
    }
    if (wthis>0.0) {
      ht[numtherm]= hp;
      Wt[numtherm]= wthis;
      Wmax = max(Wmax,wthis/1.5);
      numtherm++;
    }
  }

  if ((!draw_start_height) && (numtherm<=1)) {
    return; // don't display if insufficient statistics
    // but do draw if start height needs to be drawn
  }
  
  // drawing info
  

  // position of thermal band
  if (numtherm>1) {
    const auto hpOld = Surface.SelectObject(hpThermalBand);
    const auto hbOld = Surface.SelectObject(LKBrush_Emerald);
 
    POINT ThermalProfile[NUMTHERMALBUCKETS+3];
    for (unsigned i=0; i<numtherm; ++i) {    
        ThermalProfile[1+i].x = 
              (iround((Wt[i]/Wmax)*IBLSCALE(TBSCALEX)))+lkvariooffset; //@ 091118
      
        ThermalProfile[1+i].y = 
              NIBLSCALE(4)+iround(TBSCALEY*(1.0-ht[i]))+rc.top;
    }
    ThermalProfile[0].x = lkvariooffset;
    ThermalProfile[0].y = ThermalProfile[1].y;
    ThermalProfile[numtherm+1].x = lkvariooffset; //@ 091118
    ThermalProfile[numtherm+1].y = ThermalProfile[numtherm].y;
    ThermalProfile[numtherm+2] = ThermalProfile[0];

    Surface.Polygon(ThermalProfile,numtherm+3);
    Surface.SelectObject(hpOld);
    Surface.SelectObject(hbOld);
  }
    
  // position of thermal band

  GliderBand[0].x += lkvariooffset; // 091123 added
  GliderBand[0].y = NIBLSCALE(4)+iround(TBSCALEY*(1.0-hglider))+rc.top;
  GliderBand[1].y = GliderBand[0].y;
  GliderBand[1].x = max(iround((mc/Wmax)*IBLSCALE(TBSCALEX)),NIBLSCALE(4)) +lkvariooffset - NIBLSCALE(4); //@ 091118 rc.left

  GliderBand[2].x = GliderBand[1].x-NIBLSCALE(4);
  GliderBand[2].y = GliderBand[0].y-NIBLSCALE(4);
  GliderBand[3].x = GliderBand[1].x + NIBLSCALE(4);
  GliderBand[3].y = GliderBand[1].y;
  GliderBand[4].x = GliderBand[1].x-NIBLSCALE(4);
  GliderBand[4].y = GliderBand[0].y+NIBLSCALE(4);
  GliderBand[5].x = GliderBand[2].x;
  GliderBand[5].y = GliderBand[2].y;

  const auto hpOld = Surface.SelectObject(hpThermalBandGlider);
  Surface.Polyline(GliderBand, 2);
  
  Surface.SelectObject(LK_NULL_PEN);
  const auto hbOld = Surface.SelectObject(LK_BLACK_BRUSH);
  
  Surface.Polygon(GliderBand+2, 4);

  if (draw_start_height) {
    Surface.SelectObject(hpFinalGlideBelow);
    GliderBand[0].y = NIBLSCALE(4)+iround(TBSCALEY*(1.0-hstart))+rc.top;
    GliderBand[1].y = GliderBand[0].y;
    Surface.Polyline(GliderBand, 2);
  }

  Surface.SelectObject(hpOld);
  Surface.SelectObject(hbOld);
  
  ThermalBarDrawn=true;
}

