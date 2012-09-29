/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Terrain.cpp,v 8.6 2010/12/17 02:02:27 root Exp root $
*/

#include "externs.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "STScreenBuffer.h"
#include "RGB.h"

// #define TDEBUG 1

#if RASTERCACHE
#define USERASTERCACHE 1
#endif

#define NUM_COLOR_RAMP_LEVELS 13
// If you change NUMRAMPS, also change the InputEvent SERVICE TERRCOL 
#define NUMRAMPS        14
#include "./ColorRamps.h"


unsigned short minalt=9999;

extern bool FastZoom;

Topology* TopoStore[MAXTOPOLOGY];

BYTE tshadow_r, tshadow_g, tshadow_b, tshadow_h;
BYTE thighlight_r, thighlight_g, thighlight_b, thighlight_h;


static COLORRAMP* lastColorRamp = NULL;

void ColorRampLookup(const short h, 
                     BYTE &r, BYTE &g, BYTE &b,
		     const COLORRAMP* ramp_colors, 
                     const int numramp,
                     const unsigned char interp_levels) {

  unsigned short f, of;
  unsigned short is = 1<<interp_levels;

  // gone past end, so use last color
  if (h>=ramp_colors[numramp-1].h) {
    r = ramp_colors[numramp-1].r;
    g = ramp_colors[numramp-1].g;
    b = ramp_colors[numramp-1].b;
    return;
  }
  for (unsigned int i=numramp-2; i--; ) {
    if (h>=ramp_colors[i].h) {
      f = (unsigned short)(h-ramp_colors[i].h)*is/
        (unsigned short)(ramp_colors[i+1].h-ramp_colors[i].h);
      of = is-f;
      
      r = (f*ramp_colors[i+1].r+of*ramp_colors[i].r) >> interp_levels;
      g = (f*ramp_colors[i+1].g+of*ramp_colors[i].g) >> interp_levels;
      b = (f*ramp_colors[i+1].b+of*ramp_colors[i].b) >> interp_levels;
      return;
    }
  }

  // check if h lower than lowest
  if (h<=ramp_colors[0].h) {
    r = ramp_colors[0].r;
    g = ramp_colors[0].g;
    b = ramp_colors[0].b;
    return;
  }
}


#define MIX(x,y,i) (BYTE)((x*i+y*((1<<7)-i))>>7)


inline void TerrainShading(const short illum, BYTE &r, BYTE &g, BYTE &b)
{
  char x;
  if (illum<0) {           // shadow to blue
    x = min((int)tshadow_h,-illum);
    r = MIX(tshadow_r,r,x);
    g = MIX(tshadow_g,g,x);
    b = MIX(tshadow_b,b,x);
  } else if (illum>0) {    // highlight to yellow
    if (thighlight_h == 255) return; // 101016
    x = min((int)thighlight_h,illum/2);
    r = MIX(thighlight_r,r,x);
    g = MIX(thighlight_g,g,x);
    b = MIX(thighlight_b,b,x);
  }
}


// map scale is approximately 2 points on the grid
// therefore, want one to one mapping if mapscale is 0.5
// there are approx 30 pixels in mapscale
// 240/DTQUANT resolution = 6 pixels per terrain
// (mapscale/30)  km/pixels
//        0.250   km/terrain
// (0.25*30/mapscale) pixels/terrain
//  mapscale/(0.25*30)
//  mapscale/7.5 terrain units/pixel
// 
// this is for TerrainInfo.StepSize = 0.0025;


class TerrainRenderer {
public:
  TerrainRenderer(RECT rc) {
    #if (WINDOWSPC>0) && TESTBENCH
    StartupStore(_T(".... Init TerrainRenderer area (%d,%d) (%d,%d)\n"),rc.left,rc.top,rc.right,rc.bottom);
    #endif

    #if USERASTERCACHE
    if (!RasterTerrain::IsDirectAccess()) {
      dtquant = 6;
    } else {
	dtquant = 2; 
    }
    #else
	dtquant = 2; 
    #endif
    blursize = max((unsigned int)0, (dtquant-1)/2);
    oversampling = max(1,(blursize+1)/2+1);
    if (blursize==0) {
      oversampling = 1; // no point in oversampling, just let stretchblt do the scaling
    }

    /*
      dtq  ovs  blur  res_x  res_y   sx  sy  terrain_loads  pixels
       1    1    0    320    240    320 240    76800        76800
       2    1    0    160    120    160 120    19200        19200
       3    2    1    213    160    107  80     8560        34080
       4    2    1    160    120     80  60     4800        19200
       5    3    2    192    144     64  48     3072        27648
    */

    #if ((WINDOWSPC>0) && !TESTBENCH)
    // need at least 2Ghz singlecore CPU here for dtquant 1
    dtquant=2;
    #else
    // scale dtquant so resolution is not too high on large displays
    dtquant *= ScreenScale;  // lower resolution a bit.. (no need for CPU >800mHz)

    if (ScreenSize!=ss640x480)
	    if (dtquant>3) dtquant=3; // .. but not too much
    #endif

    int res_x = iround((rc.right-rc.left)*oversampling/dtquant);
    int res_y = iround((rc.bottom-rc.top)*oversampling/dtquant);

    sbuf = new CSTScreenBuffer();
    LKASSERT(sbuf!=NULL);
    sbuf->Create(res_x, res_y, RGB_WHITE);
    ixs = sbuf->GetCorrectedWidth()/oversampling;
    iys = sbuf->GetHeight()/oversampling;

    hBuf = (unsigned short*)malloc(sizeof(unsigned short)*ixs*iys);

    if (hBuf==NULL)  {
	StartupStore(_T("------ TerrainRenderer: malloc(%d) failed!%s"),sizeof(unsigned short)*ixs*iys, NEWLINE);
	OutOfMemory(__FILE__,__LINE__);
    } 
    #if (WINDOWSPC>0) && TESTBENCH
    else {
	StartupStore(_T(". TerrainRenderer: malloc(%d) ok%s"),sizeof(unsigned short)*ixs*iys, NEWLINE);
    }
    #endif

    colorBuf = (BGRColor*)malloc(256*128*sizeof(BGRColor));
    if (colorBuf==NULL)  {
	OutOfMemory(__FILE__,__LINE__);
    }
    // Reset this, so ColorTable will reload colors
    lastColorRamp = NULL;

  }

  ~TerrainRenderer() {
    #if (WINDOWSPC>0) && TESTBENCH
    StartupStore(_T(".... Deinit TerrainRenderer\n"));
    #endif
    if (hBuf) {
	free(hBuf);
	hBuf=NULL;
    }
    if (colorBuf) {
	free(colorBuf);
	colorBuf=NULL;
    }
    if (sbuf){
	delete sbuf;
	sbuf=NULL;
    }
  }

public:
  POINT spot_max_pt;
  POINT spot_min_pt;
  short spot_max_val;
  short spot_min_val;

private:

  unsigned int ixs, iys; // screen dimensions in coarse pixels
  unsigned int dtquant;
  unsigned int epx; // step size used for slope calculations

  RECT rect_visible;

  CSTScreenBuffer *sbuf;

  double pixelsize_d;

  int oversampling;
  int blursize;

  unsigned short *hBuf;
  BGRColor *colorBuf;
  bool do_shading;
  RasterMap *DisplayMap;
  bool is_terrain;
  int interp_levels;
  COLORRAMP* color_ramp;
  unsigned int height_scale;

public:
  bool SetMap() {
      if (hBuf==NULL || colorBuf==NULL) return false;
      interp_levels = 2;
      is_terrain = true;
      height_scale = 4;
      DisplayMap = RasterTerrain::TerrainMap;
      color_ramp = (COLORRAMP*)&terrain_colors[TerrainRamp][0];

    if (is_terrain) {
	do_shading = true;
    } else {
	do_shading = false;
    }

    if (DisplayMap) 
      return true;
    else 
      return false;

  }

  void SetShading() { 
	if (is_terrain && Shading && terrain_doshading[TerrainRamp])
		do_shading=true;
	else
		do_shading=false;
  }

  void Height() {

    double X, Y;
    int x, y; 
    int X0 = (unsigned int)(dtquant/2); 
    int Y0 = (unsigned int)(dtquant/2);
    int X1 = (unsigned int)(X0+dtquant*ixs);
    int Y1 = (unsigned int)(Y0+dtquant*iys);

    unsigned int rfact=1;

    if (QUICKDRAW) {
      #if USERASTERCACHE
      // Raster map is always DirectAccess for LK
      if (!RasterTerrain::IsDirectAccess()) {
        // first time displaying this data, so do it at half resolution
        // to avoid too many cache misses
        rfact = 2;
      }
      #else
      // reduce quantization for a fast refresh of the map
      rfact = 4;
      #endif
    }

    double pixelDX, pixelDY;

    x = (X0+X1)/2;
    y = (Y0+Y1)/2;
    MapWindow::Screen2LatLon(x, y, X, Y);
    double xmiddle = X;
    double ymiddle = Y;
    int dd = (int)lround(dtquant*rfact);

    x = (X0+X1)/2+dd;
    y = (Y0+Y1)/2;
    MapWindow::Screen2LatLon(x, y, X, Y);
    #if USERASTERCACHE
    float Xrounding = (float)fabs(X-xmiddle);
    #endif
    DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDX, NULL);

    x = (X0+X1)/2;
    y = (Y0+Y1)/2+dd;
    MapWindow::Screen2LatLon(x, y, X, Y);
    #if USERASTERCACHE
    float Yrounding = (float)fabs(Y-ymiddle);
    #endif
    DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDY, NULL);

    pixelsize_d = sqrt((pixelDX*pixelDX+pixelDY*pixelDY)/2.0);

    // OK, ready to start loading height

    DisplayMap->Lock();

    // set resolution

    #if USERASTERCACHE
    if (DisplayMap->IsDirectAccess()) {
      DisplayMap->SetFieldRounding(0,0);
    } else {
      DisplayMap->SetFieldRounding(Xrounding,Yrounding);
    }
    #else
      DisplayMap->SetFieldRounding(0,0);
    #endif

    epx = DisplayMap->GetEffectivePixelSize(&pixelsize_d,
                                            ymiddle, xmiddle);

    /*
    // We might accelerate drawing by disabling shading while quickdrawing,
    // but really this wouldnt change much the things now in terms of speed, 
    // while instead creating a confusing effect.
    if (QUICKDRAW) {
	do_shading=false;
    } else ...
    */

    if (epx> min(ixs,iys)/4) { 
      do_shading = false;
    } else {
      #ifdef TESTBENCH
      if (MapWindow::zoom.RealScale()>5.4) do_shading=false;
      #else

      #if (WINDOWSPC>0)
      if (MapWindow::zoom.RealScale()>7.2) do_shading=false;
      #else
      if (MapWindow::zoom.RealScale()>5.4) do_shading=false;
      #endif

      #endif

      //StartupStore(_T("..... Scale=%.3f RealScale=%.3f\n"),MapWindow::zoom.Scale(),MapWindow::zoom.RealScale());
    }

    POINT orig = MapWindow::GetOrigScreen();

    #if 0
    //#if DYNASCREEN
    rect_visible.left = max((long)MapWindow::DrawRect.left, (long)(MapWindow::DrawRect.left-(long)epx*dtquant))-orig.x;
    rect_visible.right = min((long)MapWindow::DrawRect.right, (long)(MapWindow::DrawRect.right+(long)epx*dtquant))-orig.x;
    rect_visible.top = max((long)MapWindow::DrawRect.top, (long)(MapWindow::DrawRect.top-(long)epx*dtquant))-orig.y;
    rect_visible.bottom = min((long)MapWindow::DrawRect.bottom, (long)(MapWindow::DrawRect.bottom+(long)epx*dtquant))-orig.y;
    #else
    rect_visible.left = max((long)MapWindow::MapRect.left, (long)(MapWindow::MapRect.left-(long)epx*dtquant))-orig.x;
    rect_visible.right = min((long)MapWindow::MapRect.right, (long)(MapWindow::MapRect.right+(long)epx*dtquant))-orig.x;
    rect_visible.top = max((long)MapWindow::MapRect.top, (long)(MapWindow::MapRect.top-(long)epx*dtquant))-orig.y;
    rect_visible.bottom = min((long)MapWindow::MapRect.bottom, (long)(MapWindow::MapRect.bottom+(long)epx*dtquant))-orig.y;
    #endif

    FillHeightBuffer(X0-orig.x, Y0-orig.y, X1-orig.x, Y1-orig.y);

    DisplayMap->Unlock();

  }

void FillHeightBuffer(const int X0, const int Y0, const int X1, const int Y1) {
    // fill the buffer
  LKASSERT(hBuf!=NULL);
  unsigned short* myhbuf = hBuf;
  #if TDEBUG
  #if (WINDOWSPC>0)
  unsigned short* hBufTop = hBuf+ixs*iys;
  #endif
  #endif

  const double PanLatitude =  MapWindow::GetPanLatitude();
  const double PanLongitude = MapWindow::GetPanLongitude();
  const double InvDrawScale = MapWindow::GetInvDrawScale()/1024.0;
  const double DisplayAngle = MapWindow::GetDisplayAngle();

  const int cost = ifastcosine(DisplayAngle);
  const int sint = ifastsine(DisplayAngle);

  const double ac2 = sint*InvDrawScale;
  const double ac3 = cost*InvDrawScale;

  #if USERASTERCACHE
  #else
  const RECT crect_visible = rect_visible;
  #endif
  minalt=9999;
  for (int y = Y0; y<Y1; y+= dtquant) {
	//int ycost = y*cost;  // REMOVE COMMENTED LINES AFTER DECEMBER 2012
	//int ysint = y*sint;
	//1: double ac1= PanLatitude- ycost*InvDrawScale;
	//2: double ac1= PanLatitude- y*cost*InvDrawScale;
	double ac1= PanLatitude- y*ac3;
	double cc1= y * ac2;

	for (int x = X0; x<X1; x+= dtquant, myhbuf++) {
		#if USERASTERCACHE
		if ((x>= rect_visible.left) &&
			(x<= rect_visible.right) &&
			(y>= rect_visible.top) &&
			(y<= rect_visible.bottom)) {
		#else
		if ((x>= crect_visible.left) &&
			(x<= crect_visible.right) &&
			(y>= crect_visible.top) &&
			(y<= crect_visible.bottom)) {
		#endif
			#if TDEBUG
			#if (WINDOWSPC>0)
			LKASSERT(myhbuf<hBufTop);
			#endif
			#endif

			//1: double Y = PanLatitude - (ycost+x*sint)*InvDrawScale;
			//2: double Y = PanLatitude - (ycost*InvDrawScale) - (x*sint*InvDrawScale)
			double Y = ac1 - x*ac2;

			//1: sums=2    mult=3
			//1: double X = PanLongitude + (x*cost-ysint)*invfastcosine(Y)*InvDrawScale;
			//2: double X = PanLongitude + (x*cost)*invfastcosine(Y)*InvDrawScale - ysint*invfastcosine(Y)*InvDrawScale;
			//3: double X = PanLongitude + x*invfastcosine(Y)*cost*InvDrawScale - y*invfastcosine(Y)*sint*InvDrawScale;
			//4: double X = PanLongitude + x*invfastcosine(Y)*ac3 - y*invfastcosine(Y)*ac2;
			//   int bc1=invfastcosine(Y);
			//5: double X = PanLongitude + x*bc1*ac3 - y*bc1*ac2;
			//6: double X = PanLongitude + x*bc1*ac3 - cc1*bc1;
			//7: double X = PanLongitude + bc1*(x*ac3) - bc1*(cc1);
			//8: double X = PanLongitude + bc1* (x*ac3 - cc1);
			double X = PanLongitude + ( invfastcosine(Y) * ((x*ac3)-cc1) );
			

			// this is setting to 0 any negative terrain value and can be a problem for dutch people
			// myhbuf cannot load negative values!
			*myhbuf = max(0, (int)DisplayMap->GetField(Y,X));
			if (*myhbuf!=TERRAIN_INVALID) {
				// if (*myhbuf>maxalt) maxalt=*myhbuf;
				if (*myhbuf<minalt) minalt=*myhbuf;
			}
		} else {
			// invisible terrain
			*myhbuf = TERRAIN_INVALID;
		}

	}
  }

  if (!terrain_minalt[TerrainRamp]) minalt=0;	//@ 101110
  if (TerrainRamp==13) {
	if (!GPS_INFO.NAVWarning) {
		if (CALCULATED_INFO.Flying) {
			minalt=(unsigned short)GPS_INFO.Altitude-150; // 500ft
		} else {
			minalt=(unsigned short)GPS_INFO.Altitude+100; // 330ft
		}
	} else {
		minalt+=150;
	}
  } 

  // StartupStore(_T("... MinAlt=%d MaxAlt=%d Multiplier=%.3f\n"),minalt,maxalt, (double)((double)maxalt/(double)(maxalt-minalt))); 
 
}

// JMW: if zoomed right in (e.g. one unit is larger than terrain
// grid), then increase the step size to be equal to the terrain
// grid for purposes of calculating slope, to avoid shading problems
// (gridding of display) This is why epx is used instead of 1
// previously.  for large zoom levels, epx=1

void Slope(const int sx, const int sy, const int sz) {

  LKASSERT(hBuf!=NULL);
  const int iepx = (int)epx;
  const unsigned int cixs=ixs;

  const unsigned int ciys = iys;
  
  const unsigned int ixsepx = cixs*epx;
  const unsigned int ixsright = cixs-1-iepx;
  const unsigned int iysbottom = ciys-iepx;
  const int hscale = max(1,(int)(pixelsize_d)); 
  const int tc = TerrainContrast;
  unsigned short *thBuf = hBuf;

  const BGRColor* oColorBuf = colorBuf+64*256;
  BGRColor* imageBuf = sbuf->GetBuffer();
  if (!imageBuf) return;

  unsigned short h;

  #if TDEBUG
  #if (WINDOWSPC>0)
  unsigned short* hBufTop = hBuf+cixs*ciys;
  #endif
  #endif

  for (unsigned int y = 0; y< iys; y++) {
	const int itss_y = ciys-1-y;
	const int itss_y_ixs = itss_y*cixs;
	const int yixs = y*cixs;
	bool ybottom=false;
	bool ytop=false;
	int p31, p32, p31s;

	if (y<iysbottom) {
		p31= iepx;
		ybottom = true;
	} else {
		p31= itss_y;
	}

	if (y >= (unsigned int) iepx) {
		p31+= iepx;
	} else {
		p31+= y;
		ytop = true;
	}
	p31s = p31*hscale;

	for (unsigned int x = 0 ; x<cixs; x++, thBuf++, imageBuf++) {

		#if TDEBUG
		#if (WINDOWSPC>0)
		LKASSERT(thBuf< hBufTop);
		#endif
		#endif

		// FIX here Netherland dutch terrain problem
		// if >=0 then the sea disappears...
		if ((h = *thBuf) != TERRAIN_INVALID ) { 
			// if (h==0 && LKWaterThreshold==0) { // no LKM coasts, and water altitude
			if (h==LKWaterThreshold) { // see above.. h cannot be -1000.. so only when LKW is 0 h can be equal
				*imageBuf = BGRColor(85,160,255); // set water color 
				continue;
			}
			h=h-minalt+1;

			int p20, p22;

			h = min(255, h>>height_scale);
			// no need to calculate slope if undefined height or sea level

			if (do_shading) {
				if (x<ixsright) {
					p20= iepx;
					p22= *(thBuf+iepx);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf+iepx< hBufTop);
					#endif
					#endif
				} else {
					int itss_x = cixs-x-2;
					p20= itss_x;
					p22= *(thBuf+itss_x);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf+itss_x< hBufTop);
					LKASSERT(thBuf+itss_x>= hBuf);
					#endif
					#endif
				} 
            
				if (x >= (unsigned int)iepx) {
					p20+= iepx;
					p22-= *(thBuf-iepx);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf-iepx>= hBuf); 
					#endif
					#endif
				} else {
					p20+= x;
					p22-= *(thBuf-x);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf-x>= hBuf);
					#endif
					#endif
				}
            
				if (ybottom) {
					p32 = *(thBuf+ixsepx);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf+ixsepx<hBufTop);
					#endif
					#endif
				} else {
					p32 = *(thBuf+itss_y_ixs);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf+itss_y_ixs<hBufTop);
					#endif
					#endif
				}

				if (ytop) {
					p32 -= *(thBuf-yixs);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf-yixs>=hBuf);
					#endif
					#endif
				} else {
					p32 -= *(thBuf-ixsepx);
					#if TDEBUG
					#if (WINDOWSPC>0)
					LKASSERT(thBuf-ixsepx>=hBuf);
					#endif
					#endif
				}
            
				if ((p22==0) && (p32==0)) {

					// slope is zero, so just look up the color
					*imageBuf = oColorBuf[h]; 

				} else {

					// p20 and p31 are never 0... so only p22 or p32 can be zero
					// if both are zero, the vector is 0,0,1 so there is no need
					// to normalise the vector
					int dd0 = p22*p31;
					int dd1 = p20*p32;
					int dd2 = p20*p31s;
              
					while (dd2>512) {
						// prevent overflow of magnitude calculation
						dd0 /= 2;
						dd1 /= 2;
						dd2 /= 2;
					}
					int mag = (dd0*dd0+dd1*dd1+dd2*dd2);
					if (mag>0) {
						mag = (dd2*sz+dd0*sx+dd1*sy)/isqrt4(mag);
						mag = max(-64,min(63,(mag-sz)*tc/128));
						*imageBuf = oColorBuf[h+mag*256];
					} else {
						*imageBuf = oColorBuf[h];
					}
				}
			} else {
				// not using shading, so just look up the color
				*imageBuf = oColorBuf[h];
			}
		} else {
			// old: we're in the water, so look up the color for water
			// new: h is TERRAIN_INVALID here
			*imageBuf = oColorBuf[255];
		}
	} // for
  } // for
};



void ColorTable() {
  if (color_ramp == lastColorRamp) {
	// no need to update the color table
	return;
  }
  lastColorRamp = color_ramp;

  for (int i=0; i<256; i++) {
	for (int mag= -64; mag<64; mag++) {
		BYTE r, g, b; 
		// i=255 means TERRAIN_INVALID. Water is colored in Slope
		if (i == 255) {
			colorBuf[i+(mag+64)*256] = BGRColor(194,223,197); // LCD green terrain invalid
		} else {
			// height_scale, color_ramp interp_levels  used only for weather
			// ColorRampLookup is preparing terrain color to pass to TerrainShading for mixing

			ColorRampLookup(i<<height_scale, r, g, b, color_ramp, NUM_COLOR_RAMP_LEVELS, interp_levels);
			TerrainShading(mag, r, g, b);
			colorBuf[i+(mag+64)*256] = BGRColor(r,g,b);
		}
	}
  }
}

  void Draw(HDC hdc, RECT rc) {

    sbuf->Zoom(oversampling);

    if (blursize>0) {

      sbuf->HorizontalBlur(blursize); 
      sbuf->VerticalBlur(blursize);

    }
    sbuf->DrawStretch(&hdc, rc);

  }

};


TerrainRenderer *trenderer = NULL;


void CloseTerrainRenderer() {
  if (trenderer) {
    delete trenderer;
    trenderer=NULL;
  }
  #if TESTBENCH
  else 
    StartupStore(_T(".... CANNOT CloseTerrainRenderer, trenderer null!!\n"));
  #endif
}


void DrawTerrain( const HDC hdc, const RECT rc, 
                  const double sunazimuth, const double sunelevation)
{
  (void)sunelevation; // TODO feature: sun-based rendering option
  (void)rc;

  if (!RasterTerrain::isTerrainLoaded()) {
    return;
  }

#if DYNASCREEN
  static RECT oldrc;
#endif
_redo:

  if (!trenderer) {
#if DYNASCREEN
    oldrc=rc;
#endif
    trenderer = new TerrainRenderer(rc);
    LKASSERT(trenderer);
    if (!trenderer) return;
  }

#if DYNASCREEN
  // Resolution has changed, probably PAN mode on with bottombar full opaque
  // We paint full screen, so we resize it.
  if (LKSW_ResetTerrainRenderer || (rc.bottom != oldrc.bottom) || (rc.right != oldrc.right)
	|| (rc.left != oldrc.left) || (rc.top != oldrc.top) ) {
    	#if (WINDOWSPC>0) && TESTBENCH
	if (LKSW_ResetTerrainRenderer) {
		StartupStore(_T("... SWITCH forced for TerrainRenderer Reset\n"));
	} else {
		StartupStore(_T("... Change vertical resolution from %d,%d,%d,%d  to %d,%d,%d,%d\n"),
			oldrc.left,oldrc.top,oldrc.right,oldrc.bottom, rc.left, rc.top, rc.right,rc.bottom);
	}
	#endif
	LKSW_ResetTerrainRenderer=false; // in any case
	CloseTerrainRenderer();
	goto _redo;
  }
#else
  if (LKSW_ResetTerrainRenderer) {
    	#if (WINDOWSPC>0) && TESTBENCH
	StartupStore(_T("... SWITCH forced for TerrainRenderer Reset\n"));
	#endif
	LKSW_ResetTerrainRenderer=false;
	CloseTerrainRenderer();
	goto _redo;
  }
#endif

  if (!trenderer->SetMap()) {
    return;
  }

  // load terrain shading parameters
  // Make them instead dynamically calculated based on previous average terrain illumination
  tshadow_r= terrain_shadow[TerrainRamp].r;
  tshadow_g= terrain_shadow[TerrainRamp].g;
  tshadow_b= terrain_shadow[TerrainRamp].b;
  tshadow_h= terrain_shadow[TerrainRamp].h;

  thighlight_r= terrain_highlight[TerrainRamp].r;
  thighlight_g= terrain_highlight[TerrainRamp].g;
  thighlight_b= terrain_highlight[TerrainRamp].b;
  thighlight_h= terrain_highlight[TerrainRamp].h;

  // step 1: calculate sunlight vector
  int sx, sy, sz;
  double fudgeelevation = (10.0+80.0*TerrainBrightness/255.0);

  sx = (int)(255*(fastcosine(fudgeelevation)*fastsine(sunazimuth)));
  sy = (int)(255*(fastcosine(fudgeelevation)*fastcosine(sunazimuth)));
  sz = (int)(255*fastsine(fudgeelevation));

  trenderer->SetShading();
  trenderer->ColorTable();
  // step 2: fill height buffer

  trenderer->Height(); 

  // step 3: calculate derivatives of height buffer
  // step 4: calculate illumination and colors
  trenderer->Slope(sx, sy, sz); 
 
  // step 5: draw
#if DYNASCREEN
  trenderer->Draw(hdc, rc);
#else
  trenderer->Draw(hdc, MapWindow::MapRect);
#endif

}

