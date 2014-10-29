/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"

void MapWindow::DrawThermalBand(LKSurface& Surface, const RECT& rc)
{
  POINT GliderBand[5] = { {0,0},{23,0},{22,0},{24,0},{0,0} };
  
  if ((DerivedDrawInfo.TaskAltitudeDifference>50)
      &&(mode.Is(Mode::MODE_FINAL_GLIDE))) {
    return;
  }

  // JMW TODO accuracy: gather proper statistics
  // note these should/may also be relative to ground
  int i;
  double mth = DerivedDrawInfo.MaxThermalHeight;
  double maxh, minh;
  double h;
  double Wt[NUMTHERMALBUCKETS];
  double ht[NUMTHERMALBUCKETS];
  double Wmax=0.0;
  int TBSCALEY = ( (rc.bottom - rc.top )/2)-NIBLSCALE(30);
#define TBSCALEX 20
  
  // calculate height above safety altitude
  double hoffset = DerivedDrawInfo.TerrainBase;
  h = DerivedDrawInfo.NavAltitude-hoffset;

  bool draw_start_height = ((ActiveWayPoint==0) && (ValidTaskPoint(0)) 
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

  short lkvariooffset;
  lkvariooffset=rc.left + LKVarioBar?(LKVarioSize+1):0; 

  // calculate top/bottom height
  maxh = max(h, mth);
  minh = min(h, 0.0);

  if (draw_start_height) {
    maxh = max(maxh, hstart);
    minh = min(minh, hstart);
  }
  
  // no thermalling has been done above safety altitude
  if (mth<=1) {
    return;
  }
  if (maxh-minh<=0) {
    return;
  }

  // normalised heights
  double hglider = (h-minh)/(maxh-minh);
  hstart = (hstart-minh)/(maxh-minh);

  // calculate averages
  int numtherm = 0;

  double mc = MACCREADY;
  Wmax = max(0.5,mc);

  for (i=0; i<NUMTHERMALBUCKETS; i++) {
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
  LKPen hpOld;
  

  // position of thermal band
  if (numtherm>1) {
    hpOld = Surface.SelectObject(hpThermalBand);
    LKBrush hbOld = Surface.SelectObject(LKBrush_Emerald);
 

    POINT ThermalProfile[NUMTHERMALBUCKETS+2];
    for (i=0; i<numtherm; i++) {    
      ThermalProfile[1+i].x = 
	(iround((Wt[i]/Wmax)*IBLSCALE(TBSCALEX)))+lkvariooffset; //@ 091118
      
      ThermalProfile[1+i].y = 
	NIBLSCALE(4)+iround(TBSCALEY*(1.0-ht[i]))+rc.top;
    }
    ThermalProfile[0].x = lkvariooffset;
    ThermalProfile[0].y = ThermalProfile[1].y;
    ThermalProfile[numtherm+1].x = lkvariooffset; //@ 091118
    ThermalProfile[numtherm+1].y = ThermalProfile[numtherm].y;

    Surface.Polygon(ThermalProfile,numtherm+2);
    Surface.SelectObject(hbOld);
  }
    
  // position of thermal band

  GliderBand[0].x += lkvariooffset; // 091123 added
  GliderBand[0].y = NIBLSCALE(4)+iround(TBSCALEY*(1.0-hglider))+rc.top;
  GliderBand[1].y = GliderBand[0].y;
  GliderBand[1].x = max(iround((mc/Wmax)*IBLSCALE(TBSCALEX)),NIBLSCALE(4)) +lkvariooffset; //@ 091118 rc.left

  GliderBand[2].x = GliderBand[1].x-NIBLSCALE(4);
  GliderBand[2].y = GliderBand[0].y-NIBLSCALE(4);
  GliderBand[3].x = GliderBand[1].x;
  GliderBand[3].y = GliderBand[1].y;
  GliderBand[4].x = GliderBand[1].x-NIBLSCALE(4);
  GliderBand[4].y = GliderBand[0].y+NIBLSCALE(4);

  hpOld = Surface.SelectObject(hpThermalBandGlider);
  
  Surface.Polyline(GliderBand, 2);
  Surface.Polyline(GliderBand+2, 3); // arrow head

  if (draw_start_height) {
    Surface.SelectObject(hpFinalGlideBelow);
    GliderBand[0].y = NIBLSCALE(4)+iround(TBSCALEY*(1.0-hstart))+rc.top;
    GliderBand[1].y = GliderBand[0].y;
    Surface.Polyline(GliderBand, 2);
  }

  Surface.SelectObject(hpOld);
  
}

