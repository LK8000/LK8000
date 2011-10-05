/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#else
#include "wcecompat/ts_string.h"
#endif

#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "lk8000.h"
#include "Utils.h"
#include "Utils2.h"
#include "device.h"
#include "uniqueid.h"
#include "Topology.h"
#include "Terrain.h"
#include "Units.h"
#include "Calculations.h"
#include "McReady.h"
#include "NavFunctions.h"
#include "WaveThread.h"
#ifdef PNA
#include "LKHolux.h"
#endif


#include "utils/heapcheck.h"


#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif


#ifdef PNA
BOOL GetFontPath(TCHAR *pPos)
{
  HKEY    hKey;
  DWORD   dwType = REG_SZ;
  DWORD dwSize = MAX_PATH;
  long    hRes;
  unsigned int i;
  for (i=0; i<dwSize; i++) {
    pPos[i]=0;
  }

  pPos[0]= '\0';
  hRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\FontPath"), 0, KEY_READ /*KEY_ALL_ACCESS*/, &hKey);
  if (hRes != ERROR_SUCCESS) {
	RegCloseKey(hKey);
	return FALSE;
  }

  dwSize *= 2;  // BUGFIX 100913 ?? to remove? check

  hRes = RegQueryValueEx(hKey, _T("FontPath"), 0, &dwType, (LPBYTE)pPos, &dwSize);

  RegCloseKey(hKey);
  if (hRes==ERROR_SUCCESS) return TRUE;
  else return FALSE;
}
#endif


void rotate(double &xin, double &yin, const double &angle)
{
  double x= xin;
  double y= yin;
  static double lastangle = 0;
  static double cost=1,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = (double)fastcosine(angle);
      sint = (double)fastsine(angle);
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}




void frotate(float &xin, float &yin, const float &angle)
{
  float x= xin;
  float y= yin;
  static float lastangle = 0;
  static float cost=1,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = (float)fastcosine(angle);
      sint = (float)fastsine(angle);
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}


void protate(POINT &pin, const double &angle)
{
  int x= pin.x;
  int y= pin.y;
  static double lastangle = 0;
  static int cost=1024,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  pin.x = (x*cost - y*sint + 512 )/1024;
  pin.y = (y*cost + x*sint + 512 )/1024;

  // round (x/b) = (x+b/2)/b;
  // b = 2; x = 10 -> (10+1)/2=5
  // b = 2; x = 11 -> (11+1)/2=6
  // b = 2; x = -10 -> (-10+1)/2=4
}


void protateshift(POINT &pin, const double &angle, 
		  const int &xs, const int &ys)
{
  int x= pin.x;
  int y= pin.y;
  static double lastangle = 0;
  static int cost=1024,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  pin.x = (x*cost - y*sint + 512 + (xs*1024))/1024;
  pin.y = (y*cost + x*sint + 512 + (ys*1024))/1024;

}


void irotatescale(int &xin, int &yin, const double &angle,
                  const double &scale, double &x, double &y)
{
  static double lastangle = 0;
  static double lastscale = 0;
  static int cost=1024,sint=0;
  if((angle != lastangle)||(scale != lastscale))
    {
      lastscale = scale/1024;
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  x = (xin*cost - yin*sint + 512)*lastscale;
  y = (yin*cost + xin*sint + 512)*lastscale;
}


void irotate(int &xin, int &yin, const double &angle)
{
  int x= xin;
  int y= yin;
  static double lastangle = 0;
  static int cost=1024,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  xin = (x*cost - y*sint + 512)/1024;
  yin = (y*cost + x*sint + 512)/1024;
}


void rotatescale(double &xin, double &yin, 
		 const double &angle, const double &scale)
{
  double x= xin;
  double y= yin;
  static double lastangle = 0;
  static double lastscale = 0;
  static double cost=1,sint=0;

  if((angle != lastangle)||(scale != lastscale))
    {
      lastangle = angle;
      lastscale = scale;
      cost = (double)fastcosine(angle)*scale;
      sint = (double)fastsine(angle)*scale;
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}


void frotatescale(float &xin, float &yin, const float &angle, const float &scale)
{
  float x= xin;
  float y= yin;
  static float lastangle = 0;
  static float lastscale = 0;
  static float cost=1,sint=0;

  if((angle != lastangle)||(scale != lastscale))
    {
      lastangle = angle;
      lastscale = scale;
      cost = (float)fastcosine(angle)*scale;
      sint = (float)fastsine(angle)*scale;
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}


double AngleLimit360(double theta) {
  while (theta>=360.0) {
    theta-= 360.0;
  }
  while (theta<0.0) {
    theta+= 360.0;
  }
  return theta;
}


double AngleLimit180(double theta) {
  while (theta>180.0) {
    theta-= 360.0;
  }
  while (theta<-180.0) {
    theta+= 360.0;
  }
  return theta;
}



double Reciprocal(double InBound)
{
  return AngleLimit360(InBound+180);
}


bool AngleInRange(double Angle0, double Angle1, double x, bool is_signed) {
  Angle0 = AngleLimit360(Angle0);
  Angle1 = AngleLimit360(Angle1);
  x = AngleLimit360(x);

  if (Angle1>= Angle0) {
    if ((x>=Angle0) && (x<= Angle1)) {
      return true;
    }
  } else {
    if (is_signed) {
      if ((x>=Angle0) || (x<= Angle1)) {
        return true;
      }
    } else {
      if ((x<=Angle0) || (x>= Angle1)) {
        return true;
      }
    }
  }
  return false;
}

// Use only for AAT bisector calculations!
double HalfAngle(double Angle0, double Angle1) {
  Angle0 = AngleLimit360(Angle0);
  Angle1 = AngleLimit360(Angle1);

  // TODO code: check/test this? thankfully only occurs in one spot in AAT
  if (Angle1>= Angle0) {
    return (Angle0+Angle1)/2;
  } else {
    return (Angle0+Angle1+360)/2;
  }
}


double BiSector(double InBound, double OutBound)
{
  double result;

  InBound = Reciprocal(InBound);

  if(InBound == OutBound)
    {
      result = Reciprocal(InBound);
    }

  else if (InBound > OutBound)
    {
      if( (InBound - OutBound) < 180)
	{
	  result = Reciprocal((InBound+OutBound)/2);
	}
      else
	{
	  result = (InBound+OutBound)/2;
	}
    }
  else
    {
      if( (OutBound - InBound) < 180)
	{
	  result = Reciprocal((InBound+OutBound)/2);
	}
      else
	{
	  result = (InBound+OutBound)/2;
	}
    }
  return result;
}




static double xcoords[64] = {
  0,			0.09801714,		0.195090322,	0.290284677,	0.382683432,	0.471396737,	0.555570233,	0.634393284,
  0.707106781,	0.773010453,	0.831469612,	0.881921264,	0.923879533,	0.956940336,	0.98078528,		0.995184727,
  1,			0.995184727,	0.98078528,		0.956940336,	0.923879533,	0.881921264,	0.831469612,	0.773010453,
  0.707106781,	0.634393284,	0.555570233,	0.471396737,	0.382683432,	0.290284677,	0.195090322,	0.09801714,
  0,			-0.09801714,	-0.195090322,	-0.290284677,	-0.382683432,	-0.471396737,	-0.555570233,	-0.634393284,
  -0.707106781,	-0.773010453,	-0.831469612,	-0.881921264,	-0.923879533,	-0.956940336,	-0.98078528,	-0.995184727,
  -1,			-0.995184727,	-0.98078528,	-0.956940336,	-0.923879533,	-0.881921264,	-0.831469612,	-0.773010453,
  -0.707106781,	-0.634393284,	-0.555570233,	-0.471396737,	-0.382683432,	-0.290284677,	-0.195090322,	-0.09801714
};

static double ycoords[64] = {
  1,			0.995184727,	0.98078528,		0.956940336,	0.923879533,	0.881921264,	0.831469612,	0.773010453,
  0.707106781,	0.634393284,	0.555570233,	0.471396737,	0.382683432,	0.290284677,	0.195090322,	0.09801714,
  0,			-0.09801714,	-0.195090322,	-0.290284677,	-0.382683432,	-0.471396737,	-0.555570233,	-0.634393284,
  -0.707106781,	-0.773010453,	-0.831469612,	-0.881921264,	-0.923879533,	-0.956940336,	-0.98078528,	-0.995184727,
  -1,			-0.995184727,	-0.98078528,	-0.956940336,	-0.923879533,	-0.881921264,	-0.831469612,	-0.773010453,
  -0.707106781,	-0.634393284,	-0.555570233,	-0.471396737,	-0.382683432,	-0.290284677,	-0.195090322,	-0.09801714,
  0,			0.09801714,		0.195090322,	0.290284677,	0.382683432,	0.471396737,	0.555570233,	0.634393284,
  0.707106781,	0.773010453,	0.831469612,	0.881921264,	0.923879533,	0.956940336,	0.98078528,		0.995184727
};


void StartArc(HDC hdc, 
	      double longitude0, double latitude0,
	      double longitude1, double latitude1, 
	      double arclength) {

  double radius, bearing;
  DistanceBearing(latitude0, longitude0, 
                  latitude1, longitude1,
                  &radius,
                  &bearing);
  double angle = 360*min(1, arclength/(2.0*3.1415926*radius));
  int i0 = (int)(bearing+angle/2);
  int i1 = (int)(bearing-angle/2);
  int i;
  if (i0<0) { i1+= 360; }
  if (i1<0) { i1+= 360; }
  if (i0>360) {i0-= 360; }
  if (i1>360) {i1-= 360; }
  i0 = i0*64/360;
  i1 = i1*64/360;
  POINT pt[2];
//  double lat, lon;
  int x=0;
  int y=0;

  if (i1<i0) {
    for (i=i0; i<64-1; i++) {
      //      MapWindow::LatLon2Screen(lon, lat, &scx, &scy);
      pt[0].x = x + (long) (radius * xcoords[i]);
      pt[0].y = y + (long) (radius * ycoords[i]);
      pt[1].x = x + (long) (radius * xcoords[i+1]);
      pt[1].y = y + (long) (radius * ycoords[i+1]);
      Polygon(hdc,pt,2);
    }
    for (i=0; i<i1-1; i++) {
      pt[0].x = x + (long) (radius * xcoords[i]);
      pt[0].y = y + (long) (radius * ycoords[i]);
      pt[1].x = x + (long) (radius * xcoords[i+1]);
      pt[1].y = y + (long) (radius * ycoords[i+1]);
      Polygon(hdc,pt,2);
    }
  } else {
    for (i=i0; i<i1-1; i++) {
      pt[0].x = x + (long) (radius * xcoords[i]);
      pt[0].y = y + (long) (radius * ycoords[i]);
      pt[1].x = x + (long) (radius * xcoords[i+1]);
      pt[1].y = y + (long) (radius * ycoords[i+1]);
      Polygon(hdc,pt,2);
    }
  }

}


int Circle(HDC hdc, long x, long y, int radius, RECT rc, bool clip, bool fill)
{
  POINT pt[65];
  unsigned int i;

  rectObj rect;
  rect.minx = x-radius;
  rect.maxx = x+radius;
  rect.miny = y-radius;
  rect.maxy = y+radius;
  rectObj rcrect;
  rcrect.minx = rc.left;
  rcrect.maxx = rc.right;
  rcrect.miny = rc.top;
  rcrect.maxy = rc.bottom;

  if (msRectOverlap(&rect, &rcrect)!=MS_TRUE) {
    return FALSE;
  }
  // JMW added faster checking...

  unsigned int step = 1;
  if (radius<20) {
    step = 2;
  }
  for(i=64/step;i--;) {
    pt[i].x = x + (long) (radius * xcoords[i*step]);
    pt[i].y = y + (long) (radius * ycoords[i*step]);
  }
  step = 64/step;
  pt[step].x = x + (long) (radius * xcoords[0]);
  pt[step].y = y + (long) (radius * ycoords[0]);

  if (clip) {
    ClipPolygon(hdc,pt,step+1,rc, fill);
  } else {
    if (fill) {
#ifdef PNA
    if (needclipping==true)
      	ClipPolygon(hdc,pt,step+1,rc, true); // VNT10 090909 fixed bug was false FIX CHECK IF WORKING 
    else
        Polygon(hdc,pt,step+1);
#else
      Polygon(hdc,pt,step+1); 
#endif 
    } else {
// VENTA3 FIX HP clipping bug
#ifdef PNA
      if (needclipping==true)
      	MapWindow::_Polyline(hdc,pt,step+1,rc);
      else
        Polyline(hdc,pt,step+1);
#else
      Polyline(hdc,pt,step+1);
#endif
    }
  }
  return TRUE;
}


int Segment(HDC hdc, long x, long y, int radius, RECT rc, 
	    double start,
	    double end,
            bool horizon)
{
  POINT pt[66];
  int i;
  int istart;
  int iend;

  rectObj rect;
  rect.minx = x-radius;
  rect.maxx = x+radius;
  rect.miny = y-radius;
  rect.maxy = y+radius;
  rectObj rcrect;
  rcrect.minx = rc.left;
  rcrect.maxx = rc.right;
  rcrect.miny = rc.top;
  rcrect.maxy = rc.bottom;

  if (msRectOverlap(&rect, &rcrect)!=MS_TRUE) {
    return FALSE;
  }

  // JMW added faster checking...

  start = AngleLimit360(start);
  end = AngleLimit360(end);

  istart = iround(start/360.0*64);
  iend = iround(end/360.0*64);

  int npoly = 0;

  if (istart>iend) {
    iend+= 64;
  }
  istart++;
  iend--;

  if (!horizon) {
    pt[0].x = x; pt[0].y = y; npoly=1;
  }
  pt[npoly].x = x + (long) (radius * fastsine(start));
  pt[npoly].y = y - (long) (radius * fastcosine(start));
  npoly++;

  for(i=0;i<64;i++) {
    if (i<=iend-istart) {
      pt[npoly].x = x + (long) (radius * xcoords[(i+istart)%64]);
      pt[npoly].y = y - (long) (radius * ycoords[(i+istart)%64]);
      npoly++;
    }
  }
  pt[npoly].x = x + (long) (radius * fastsine(end));
  pt[npoly].y = y - (long) (radius * fastcosine(end));
  npoly++;

  if (!horizon) {
    pt[npoly].x = x; 
    pt[npoly].y = y; npoly++;
  } else {
    pt[npoly].x = pt[0].x;
    pt[npoly].y = pt[0].y;
    npoly++;
  }
  if (npoly) {
    Polygon(hdc,pt,npoly);
  }
  
  return TRUE;
}

/*
 * VENTA3 This is a modified Segment() 
 */
int DrawArc(HDC hdc, long x, long y, int radius, RECT rc, 
	    double start,
	    double end)
{
  POINT pt[66];
  int i;
  int istart;
  int iend;

  rectObj rect;
  rect.minx = x-radius;
  rect.maxx = x+radius;
  rect.miny = y-radius;
  rect.maxy = y+radius;
  rectObj rcrect;
  rcrect.minx = rc.left;
  rcrect.maxx = rc.right;
  rcrect.miny = rc.top;
  rcrect.maxy = rc.bottom;

  if (msRectOverlap(&rect, &rcrect)!=MS_TRUE) {
    return FALSE;
  }

  // JMW added faster checking...

  start = AngleLimit360(start);
  end = AngleLimit360(end);

  istart = iround(start/360.0*64);
  iend = iround(end/360.0*64);

  int npoly = 0;

  if (istart>iend) {
    iend+= 64;
  }
  istart++;
  iend--;

  pt[npoly].x = x + (long) (radius * fastsine(start));
  pt[npoly].y = y - (long) (radius * fastcosine(start));
  npoly++;

  for(i=0;i<64;i++) {
    if (i<=iend-istart) {
      pt[npoly].x = x + (long) (radius * xcoords[(i+istart)%64]);
      pt[npoly].y = y - (long) (radius * ycoords[(i+istart)%64]);
      npoly++;
    }
  }
  pt[npoly].x = x + (long) (radius * fastsine(end));
  pt[npoly].y = y - (long) (radius * fastcosine(end));
  npoly++;
  if (npoly) {
    Polyline(hdc,pt,npoly); // TODO check ClipPolygon for HP31X
  }
  
  return TRUE;
}

BOOL PolygonVisible(const POINT *lpPoints, int nCount, RECT rc)
{
  BOOL Sector[9] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
  int i;
  int Count = 0;
  (void)rc;
  //return TRUE;

  for(i=0;i<nCount;i++)
    {
      if(lpPoints[i].y < MapWindow::MapRect.top)
	{
	  if(lpPoints[i].x < MapWindow::MapRect.left)
	    {
	      Sector[0] = TRUE;
	    }
	  else if((lpPoints[i].x >=MapWindow::MapRect.left) 
		  && (lpPoints[i].x <MapWindow::MapRect.right))
	    {
	      Sector[1] = TRUE;
	    }
	  else if(lpPoints[i].x >=MapWindow::MapRect.right)
	    {
	      Sector[2] = TRUE;
	    }
	}
      else if((lpPoints[i].y >=MapWindow::MapRect.top) 
	      && (lpPoints[i].y <MapWindow::MapRect.bottom))
	{
	  if(lpPoints[i].x <MapWindow::MapRect.left)
	    {
	      Sector[3] = TRUE;
	    }
	  else if((lpPoints[i].x >=MapWindow::MapRect.left) 
		  && (lpPoints[i].x <MapWindow::MapRect.right))
	    {
	      Sector[4] = TRUE;
	      return TRUE;
	    }
	  else if(lpPoints[i].x >=MapWindow::MapRect.right)
	    {
	      Sector[5] = TRUE;
	    }
	}
      else if(lpPoints[i].y >=MapWindow::MapRect.bottom)
	{
	  if(lpPoints[i].x <MapWindow::MapRect.left)
	    {
	      Sector[6] = TRUE;
	    }
	  else if((lpPoints[i].x >=MapWindow::MapRect.left) 
		  && (lpPoints[i].x <MapWindow::MapRect.right))
	    {
	      Sector[7] = TRUE;
	    }
	  else if(lpPoints[i].x >=MapWindow::MapRect.right)
	    {
	      Sector[8] = TRUE;
	    }
	}
    }

  for(i=0;i<9;i++)
    {
      if(Sector[i])
	{
	  Count ++;
	}
    }

  if(Count>= 2)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}


double ScreenAngle(int x1, int y1, int x2, int y2)
{
  return atan2((double)y2-y1, (double)x2-x1)*RAD_TO_DEG;
}

void FormatWarningString(int Type, TCHAR *Name , AIRSPACE_ALT Base, AIRSPACE_ALT Top, TCHAR *szMessageBuffer, TCHAR *szTitleBuffer )
{
  TCHAR BaseStr[512];
  TCHAR TopStr[512];

  switch (Type)
    {
    case RESTRICT:	  
	// LKTOKEN  _@M565_ = "Restricted" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M565_"))); break;
    case PROHIBITED:	  
	// LKTOKEN  _@M537_ = "Prohibited" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M537_"))); break;
    case DANGER:          
	// LKTOKEN  _@M213_ = "Danger Area" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M213_"))); break;
    case CLASSA:          
      _tcscpy(szTitleBuffer,TEXT("Class A")); break;
    case CLASSB:          
      _tcscpy(szTitleBuffer,TEXT("Class B")); break;
    case CLASSC:          
      _tcscpy(szTitleBuffer,TEXT("Class C")); break;
    case CLASSD:          
      _tcscpy(szTitleBuffer,TEXT("Class D")); break;
    case CLASSE:			
      _tcscpy(szTitleBuffer,TEXT("Class E")); break;
    case CLASSF:			
      _tcscpy(szTitleBuffer,TEXT("Class F")); break;
    case CLASSG:			
      _tcscpy(szTitleBuffer,TEXT("Class G")); break;
    case NOGLIDER:		
	// LKTOKEN  _@M464_ = "No Glider" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M464_"))); break;
    case CTR:					
      _tcscpy(szTitleBuffer,TEXT("CTR")); break;
    case WAVE:				
	// LKTOKEN  _@M794_ = "Wave" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M794_"))); break;
    case CLASSTMZ:            
      _tcscpy(szTitleBuffer,TEXT("TMZ")); break;
    default:					
	// LKTOKEN  _@M765_ = "Unknown" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M765_")));
    }

  if(Base.FL == 0)
    {
      if (Base.AGL > 0) {
        _stprintf(BaseStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Base.AGL, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("AGL"));
      } else if (Base.Altitude > 0)
        _stprintf(BaseStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Base.Altitude, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("MSL"));
      else
        _stprintf(BaseStr,TEXT("GND"));
    }
  else
    {
      _stprintf(BaseStr,TEXT("FL %1.0f"),Base.FL );
    }

  if(Top.FL == 0)
    {
      if (Top.AGL > 0) {
        _stprintf(TopStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Top.AGL, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("AGL"));
      } else {
	_stprintf(TopStr,TEXT("%1.0f%s %s"), ALTITUDEMODIFY * Top.Altitude, 
		  Units::GetUnitName(Units::GetUserAltitudeUnit()),
		  TEXT("MSL"));
      }
    }
  else
    {
      _stprintf(TopStr,TEXT("FL %1.0f"),Top.FL );
    }

  _stprintf(szMessageBuffer,TEXT("%s: %s\r\n%s: %s\r\n%s: %s\r\n"),
            szTitleBuffer, 
            Name, 
	// LKTOKEN  _@M729_ = "Top" 
            gettext(TEXT("_@M729_")),
            TopStr,
	// LKTOKEN  _@M128_ = "Base" 
            gettext(TEXT("_@M128_")),
            BaseStr 
            );
}



void InitSineTable(void)
{
  int i;
  double angle;
  double cosa, sina;

  for(i=0;i<4096; i++)
    {
      angle = DEG_TO_RAD*((double)i*360)/4096;
      cosa = cos(angle);
      sina = sin(angle);
      SINETABLE[i] = sina;
      COSTABLE[i] = cosa;
      ISINETABLE[i] = iround(sina*1024);
      ICOSTABLE[i] = iround(cosa*1024);
      if ((cosa>0) && (cosa<1.0e-8)) {
	cosa = 1.0e-8;
      }
      if ((cosa<0) && (cosa>-1.0e-8)) {
	cosa = -1.0e-8;
      }
      INVCOSINETABLE[i] = 1.0/cosa;
    }
}


unsigned int isqrt4(unsigned long val) {
  unsigned int temp, g=0;

  if (val >= 0x40000000) {
    g = 0x8000;
    val -= 0x40000000;
  }

#define INNER_MBGSQRT(s)                      \
  temp = (g << (s)) + (1 << ((s) * 2 - 2));   \
  if (val >= temp) {                          \
    g += 1 << ((s)-1);                        \
    val -= temp;                              \
  }

  INNER_MBGSQRT (15)
  INNER_MBGSQRT (14)
  INNER_MBGSQRT (13)
  INNER_MBGSQRT (12)
  INNER_MBGSQRT (11)
  INNER_MBGSQRT (10)
  INNER_MBGSQRT ( 9)
  INNER_MBGSQRT ( 8)
  INNER_MBGSQRT ( 7)
  INNER_MBGSQRT ( 6)
  INNER_MBGSQRT ( 5)
  INNER_MBGSQRT ( 4)
  INNER_MBGSQRT ( 3)
  INNER_MBGSQRT ( 2)

#undef INNER_MBGSQRT

  temp = g+g+1;
  if (val >= temp) g++;
  return g;
}

// http://www.azillionmonkeys.com/qed/sqroot.html




static int ByteCRC16(int value, int crcin)
{
    int k = (((crcin >> 8) ^ value) & 255) << 8;
    int crc = 0;
    int bits = 8;
    do
    {
        if (( crc ^ k ) & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
        k <<= 1;
    }
    while (--bits);
    return ((crcin << 8) ^ crc);
}

WORD crcCalc(void *Buffer, size_t size){

  int crc = 0;
  unsigned char *pB = (unsigned char *)Buffer;

  do {
    int value = *pB++;
    crc = ByteCRC16(value, crc);
  } while (--size);

  return((WORD)crc);
}

void ExtractDirectory(TCHAR *Dest, TCHAR *Source) {
  int len = _tcslen(Source);
  int found = -1;
  int i;
  if (len==0) {
    Dest[0]= 0;
    return;
  }
  for (i=0; i<len; i++) {
    if ((Source[i]=='/')||(Source[i]=='\\')) {
      found = i;
    }
  }
  for (i=0; i<=found; i++) {
    Dest[i]= Source[i];
  }
  Dest[i]= 0;
}


/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. [rescinded 22 July 1999]
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Perform a binary search.
 *
 * The code below is a bit sneaky.  After a comparison fails, we
 * divide the work in half by moving either left or right. If lim
 * is odd, moving left simply involves halving lim: e.g., when lim
 * is 5 we look at item 2, so we change lim to 2 so that we will
 * look at items 0 & 1.  If lim is even, the same applies.  If lim
 * is odd, moving right again involes halving lim, this time moving
 * the base up one item past p: e.g., when lim is 5 we change base
 * to item 3 and make lim 2 so that we will look at items 3 and 4.
 * If lim is even, however, we have to shrink it by one before
 * halving: e.g., when lim is 4, we still looked at item 2, so we
 * have to make lim 3, then halve, obtaining 1, so that we will only
 * look at item 3.
 */

void *bsearch(void *key, void *base0, size_t nmemb, size_t size, int (*compar)(const void *elem1, const void *elem2)){
	void *base = base0;
	int lim, cmp;
	void *p;

	for (lim = nmemb; lim != 0; lim >>= 1) {
		p = (char *)base + (lim >> 1) * size;
		cmp = (*compar)(key, p);
		if (cmp == 0)
			return (p);
		if (cmp > 0) {	/* key > p: move right */
			base = (char *)p + size;
			lim--;
		} /* else move left */
	}
	return (NULL);
}



void StatusFileInit() {
  #if TESTBENCH
  StartupStore(TEXT(". StatusFileInit%s"),NEWLINE);
  #endif

  // DEFAULT - 0 is loaded as default, and assumed to exist
  StatusMessageData[0].key = TEXT("DEFAULT");
  StatusMessageData[0].doStatus = true;
  StatusMessageData[0].doSound = true; 
  StatusMessageData[0].sound = TEXT("IDR_WAV_DRIP");
  StatusMessageData_Size=1;
#ifdef VENTA_DEBUG_EVENT // VENTA- longer statusmessage delay in event debug mode
	StatusMessageData[0].delay_ms = 10000;  // 10 s
#else
    StatusMessageData[0].delay_ms = 2500; // 2.5 s
#endif

  // Load up other defaults - allow overwrite in config file
#include "Status_defaults.cpp"

}

void ReadStatusFile() {
  
  StartupStore(TEXT(". Loading status file%s"),NEWLINE);

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  FILE *fp=NULL;
  
  // Open file from registry
  GetRegistryString(szRegistryStatusFile, szFile1, MAX_PATH);
  ExpandLocalPath(szFile1);

  SetRegistryString(szRegistryStatusFile, TEXT("\0"));
  
  if (_tcslen(szFile1)>0)
    fp  = _tfopen(szFile1, TEXT("rt"));
  
  // Unable to open file
  if (fp == NULL)
    return;
  
  // TODO code: Safer sizes, strings etc - use C++ (can scanf restrict length?)
  TCHAR buffer[2049];	// Buffer for all
  TCHAR key[2049];	// key from scanf
  TCHAR value[2049];	// value from scanf
  int ms;				// Found ms for delay
  TCHAR **location;	// Where to put the data
  int found;			// Entries found from scanf
  bool some_data;		// Did we find some in the last loop...
  
  // Init first entry
  _init_Status(StatusMessageData_Size);
  some_data = false;
  
  /* Read from the file */
  while (
	 (StatusMessageData_Size < MAXSTATUSMESSAGECACHE)
	 && fgetws(buffer, 2048, fp)
	 && ((found = swscanf(buffer, TEXT("%[^#=]=%[^\n]\n"), key, value)) != EOF)
	 ) {
    // Check valid line? If not valid, assume next record (primative, but works ok!)
    if ((found != 2) || key[0] == '\0' || value == '\0') {
      
      // Global counter (only if the last entry had some data)
      if (some_data) {
	StatusMessageData_Size++;
	some_data = false;
	_init_Status(StatusMessageData_Size);
      }
      
    } else {
      
      location = NULL;
      
      if (wcscmp(key, TEXT("key")) == 0) {
	some_data = true;	// Success, we have a real entry
	location = &StatusMessageData[StatusMessageData_Size].key;
      } else if (wcscmp(key, TEXT("sound")) == 0) {
	StatusMessageData[StatusMessageData_Size].doSound = true;
	location = &StatusMessageData[StatusMessageData_Size].sound;
      } else if (wcscmp(key, TEXT("delay")) == 0) {
	if (swscanf(value, TEXT("%d"), &ms) == 1)
	  StatusMessageData[StatusMessageData_Size].delay_ms = ms;
      } else if (wcscmp(key, TEXT("hide")) == 0) {
	if (wcscmp(value, TEXT("yes")) == 0)
	  StatusMessageData[StatusMessageData_Size].doStatus = false;
      }
      
      // Do we have somewhere to put this && is it currently empty ? (prevent lost at startup)
      if (location && (wcscmp(*location, TEXT("")) == 0)) {
	// TODO code: this picks up memory lost from no entry, but not duplicates - fix.
	if (*location) {
	  // JMW fix memory leak
	  free(*location);
	}
	*location = StringMallocParse(value);
      }
    }
    
  }
  
  // How many we really got (blank next just in case)
  StatusMessageData_Size++;
  _init_Status(StatusMessageData_Size);
  
  // file was ok, so save it to registry
  ContractLocalPath(szFile1);
  SetRegistryString(szRegistryStatusFile, szFile1);
  
  fclose(fp);
}

// Create a blank entry (not actually used)
void _init_Status(int num) {
	StatusMessageData[num].key = TEXT("");
	StatusMessageData[num].doStatus = true;
	StatusMessageData[num].doSound = false;
	StatusMessageData[num].sound = TEXT("");
	StatusMessageData[num].delay_ms = 2500;  // 2.5 s
}




//
//	Get local My Documents path - optionally include file to add and location
//	loc = CSIDL_PROGRAMS
//	File system directory that contains the user's program groups (which are also file system directories).
//	CSIDL_PERSONAL               File system directory that serves as a common
//	                             repository for documents.
//	CSIDL_PROGRAM_FILES 0x0026   The program files folder.
//
void LocalPath(TCHAR* buffer, const TCHAR* file, int loc) {

  #if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )

  // For PNAs the localpath is taken from the application exec path
  // example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\LK8000
  // Then the basename is searched for an underscore char, which is
  // used as a separator for getting the model type.  example>
  // program_pna.exe results in GlobalModelType=pna
  
	_stprintf(buffer,TEXT("%s%S"),gmfpathname(), XCSDATADIR );
  #else
	//
	// Windows PC environment
	//
	// Do we have a valid _System/_SYSTEM locally?
	extern TCHAR *gmfcurrentpath();
	_stprintf(buffer,_T("%s\\%S\\_SYSTEM"),gmfcurrentpath(),LKD_SYSTEM);
	if (  GetFileAttributes(buffer) != 0xffffffff )  {
		// Yes, so we use the current path folder
		_tcscpy(buffer,gmfcurrentpath());
	} else {
		// No, we use MyDocuments directory
		SHGetSpecialFolderPath(hWndMainWindow, buffer, loc, false);

		_tcscat(buffer,TEXT("\\"));
		_tcscat(buffer,TEXT(XCSDATADIR));
	}
  #endif

  if (_tcslen(file)>0) {
	_tcsncat(buffer, TEXT("\\"), MAX_PATH);    
	_tcsncat(buffer, file, MAX_PATH);
  }
}


void LocalPathS(char *buffer, const TCHAR* file, int loc) {
  TCHAR wbuffer[MAX_PATH];
  LocalPath(wbuffer,file,loc);
  sprintf(buffer,"%S",wbuffer);
}


void ExpandLocalPath(TCHAR* filein) {
  // Convert %LOCALPATH% to Local Path

  if (_tcslen(filein)==0) {
    return;
  }

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = TEXT("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];
  LocalPath(lpath);

  TCHAR* ptr;
  ptr = _tcsstr(filein, code);
  if (!ptr) return;

  ptr += _tcslen(code);
  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),lpath, ptr);
    _tcscpy(filein, output);
  }
}


void ContractLocalPath(TCHAR* filein) {
  // Convert Local Path part to %LOCALPATH%

  if (_tcslen(filein)==0) {
    return;
  }

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = TEXT("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];
  LocalPath(lpath);

  TCHAR* ptr;
  ptr = _tcsstr(filein, lpath);
  if (!ptr) return;

  ptr += _tcslen(lpath);
  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),code, ptr);
    _tcscpy(filein, output);
  }
}


int propGetScaleList(double *List, size_t Size){

  TCHAR Buffer[128];
  TCHAR Name[] = TEXT("ScaleList");
  TCHAR *pWClast, *pToken;
  int   Idx = 0;
  double vlast=0;
  double val;

  ASSERT(List != NULL);
  ASSERT(Size > 0);

  SetRegistryString(TEXT("ScaleList"),
   TEXT("0.5,1,2,5,10,20,50,100,150,200,500,1000"));

  if (GetRegistryString(Name, Buffer, sizeof(Buffer)/sizeof(TCHAR)) == 0){

    pToken = strtok_r(Buffer, TEXT(","), &pWClast);
    
    while(Idx < (int)Size && pToken != NULL){
      val = _tcstod(pToken, NULL);
      if (Idx>0) {
        List[Idx] = (val+vlast)/2;
        Idx++;
      }
      List[Idx] = val;
      Idx++;
      vlast = val;
      pToken = strtok_r(NULL, TEXT(","), &pWClast);
    }
    
    return(Idx);
    
  } else {
    return(0);
  }
  
}

long GetUTCOffset(void) {
  return UTCOffset;
}


void LK8000GetOpts(LPTSTR CommandLine) {
  (void)CommandLine;

  TCHAR buffer[MAX_PATH];
#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(XCSPROFILE)); // 091101

#else
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false);
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(XCSDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_CONF)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(XCSPROFILE)); // 091101
#endif
  _tcscpy(defaultProfileFile,buffer);
  _tcscpy(startProfileFile, defaultProfileFile);

#if (WINDOWSPC>0) 
  SCREENWIDTH=800;
  SCREENHEIGHT=480;

#if defined(SCREENWIDTH_)
  SCREENWIDTH=SCREENWIDTH_;
#endif
#if defined(SCREENHEIGHT_)
  SCREENHEIGHT=SCREENHEIGHT_;
#endif

#else
  return; // don't do anything for PDA platforms
#endif

  TCHAR *MyCommandLine = GetCommandLine();

  if (MyCommandLine != NULL){
    TCHAR *pC, *pCe;

    pC = _tcsstr(MyCommandLine, TEXT("-profile="));
    if (pC != NULL){
      pC += strlen("-profile=");
      if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
      } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
      }
      if (pCe != NULL && pCe-1 > pC){

        _tcsncpy(startProfileFile, pC, pCe-pC);
        startProfileFile[pCe-pC] = '\0';
      }
    }
#if (WINDOWSPC>0) 
    pC = _tcsstr(MyCommandLine, TEXT("-640x480"));
    if (pC != NULL){
      SCREENWIDTH=640;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x480"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-720x408"));
    if (pC != NULL){
      SCREENWIDTH=720;
      SCREENHEIGHT=408;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x600"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=600;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-896x672"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-big"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-400x240"));
    if (pC != NULL){
      SCREENWIDTH=400;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x272"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=272;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x234"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x800"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=800;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-portrait"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x640"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-small"));
    if (pC != NULL){
      SCREENWIDTH/= 2;
      SCREENHEIGHT/= 2;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x240"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x234"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=320;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-234x320"));
    if (pC != NULL){
      SCREENWIDTH=234;
      SCREENHEIGHT=320;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x400"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=400;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-272x480"));
    if (pC != NULL){
      SCREENWIDTH=272;
      SCREENHEIGHT=480;
    }

#endif
  }
}


#if TOPOFASTLABEL
bool CheckRectOverlap(const RECT *rc1, const RECT *rc2) {
  if(rc1->left >= rc2->right) return(false);
  if(rc1->right <= rc2->left) return(false);
  if(rc1->top >= rc2->bottom) return(false);
  if(rc1->bottom <= rc2->top) return(false);
  return(true);
}
#else
bool CheckRectOverlap(RECT rc1, RECT rc2) {
  if(rc1.left >= rc2.right) return(false);
  if(rc1.right <= rc2.left) return(false);
  if(rc1.top >= rc2.bottom) return(false);
  if(rc1.bottom <= rc2.top) return(false);
  return(true);
}
#endif


#if (WINDOWSPC>0)
typedef DWORD (_stdcall *GetIdleTimeProc) (void);
GetIdleTimeProc GetIdleTime;
#endif

#if DEBUG_MEM
int MeasureCPULoad() {
#if (WINDOWSPC>0) && !defined(__MINGW32__)
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    GetIdleTime = (GetIdleTimeProc) 
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("GetIdleTime"));
    init=true;
  }
  if (!GetIdleTime) return 0;
#endif

  static int pi;
  static int PercentIdle;
  static int PercentLoad;
  static bool start=true;
  static DWORD dwStartTick;
  static DWORD dwIdleSt;
  static DWORD dwStopTick;
  static DWORD dwIdleEd;
  if (start) {
    dwStartTick = GetTickCount();
    dwIdleSt = GetIdleTime();
  }
  if (!start) {
    dwStopTick = GetTickCount();
    dwIdleEd = GetIdleTime();
    pi = ((100 * (dwIdleEd - dwIdleSt))/(dwStopTick - dwStartTick));
    PercentIdle = (PercentIdle+pi)/2;
  }
  start = !start;
  PercentLoad = 100-PercentIdle;
  return PercentLoad;
}
#endif


#if (WINDOWSPC<1)
#define GdiFlush() do { } while (0)
#endif


#if 0 // REMOVE ANIMATION
static RECT AnimationRectangle = {0,0,0,0};

void SetSourceRectangle(RECT fromRect) {
  AnimationRectangle = fromRect;
}


RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed)
{
    return AnimationRectangle;
}
#endif 


unsigned long CheckFreeRam(void) {
  MEMORYSTATUS    memInfo;
  // Program memory
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatus(&memInfo);

  //	   memInfo.dwTotalPhys, 
  //	   memInfo.dwAvailPhys, 
  //	   memInfo.dwTotalPhys- memInfo.dwAvailPhys);

  return memInfo.dwAvailPhys;
}

// check maximum allocatable heap block
unsigned long CheckMaxHeapBlock(void) {
  #if defined(HC_DMALLOC) ||  defined(HC_DUMA)
    // when using heap checker, do not try allocate maximum size - malloc() can
    // return NULL which heap checker recognizes as an error and will terminate
    // program immediately when configured so (can be confusing for developer)
    return(0xFFFFFFFF);
  #else  
    // try allocate maximum block (of course on PC with disk swapping, we will not
    // try maximum block, function just returns something near to initial top value)
    size_t top = 100*1024*1024; // start with 100MB/2
    size_t btm = 0;
    
    void*  addr;
    size_t size;
    
    while ((size = (btm + top) / 2) != 0) { // ~ btm + (top - btm) / 2
      addr = malloc(size);
      if (addr == NULL)
        top = size;
      else {
        free(addr);
        if ((top - btm) < 1024) // 1 KB accuracy
          return(size);
        btm = size;
      }
    }
    
    return(0);
  #endif
}


#if (WINDOWSPC>0)
#if _DEBUG
_CrtMemState memstate_s1;
#endif
#endif

void MemCheckPoint()
{
#if (WINDOWSPC>0)
#if _DEBUG
  _CrtMemCheckpoint( &memstate_s1 );
#endif
#endif
}


void MemLeakCheck() {
#if (WINDOWSPC>0)
#if _DEBUG
  _CrtMemState memstate_s2, memstate_s3;

   // Store a 2nd memory checkpoint in s2
   _CrtMemCheckpoint( &memstate_s2 );

   if ( _CrtMemDifference( &memstate_s3, &memstate_s1, &memstate_s2 ) ) {
     _CrtMemDumpStatistics( &memstate_s3 );
     _CrtMemDumpAllObjectsSince(&memstate_s1);
   }

  _CrtCheckMemory();
#endif
#endif
}


// This is necessary to be called periodically to get rid of 
// memory defragmentation, since on pocket pc platforms there is no
// automatic defragmentation.
void MyCompactHeaps() {
#if (WINDOWSPC>0)
  HeapCompact(GetProcessHeap(),0);
#else
  typedef DWORD (_stdcall *CompactAllHeapsFn) (void);
  static CompactAllHeapsFn CompactAllHeaps = NULL;
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    CompactAllHeaps = (CompactAllHeapsFn) 
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("CompactAllHeaps"));
    init=true;
  }
  if (CompactAllHeaps) {
    CompactAllHeaps();
  }
#endif
}


unsigned long FindFreeSpace(const TCHAR *path) {
  // returns number of kb free on destination drive

  ULARGE_INTEGER FreeBytesAvailableToCaller;
  ULARGE_INTEGER TotalNumberOfBytes;
  ULARGE_INTEGER TotalNumberOfFreeBytes;
  if (GetDiskFreeSpaceEx(path, 
			 &FreeBytesAvailableToCaller,
			 &TotalNumberOfBytes,
			 &TotalNumberOfFreeBytes)) {
    return FreeBytesAvailableToCaller.LowPart/1024;
  } else {
    return 0;
  }
}




#include "mmsystem.h"

extern HINSTANCE                       hInst; // The current instance

BOOL PlayResource (const TCHAR* lpName)
{
#ifdef DISABLEAUDIO
  return false;
#else
  #ifdef PNA
  if (DeviceIsGM130) {
	MessageBeep(0xffffffff);
	return true;
  }
  #endif
  BOOL bRtn;
  LPTSTR lpRes;
  HANDLE hResInfo, hRes;

  // TODO code: Modify to allow use of WAV Files and/or Embedded files

  if (_tcsstr(lpName, TEXT(".wav"))) {
    bRtn = sndPlaySound (lpName, SND_ASYNC | SND_NODEFAULT ); 

  } else {
    
    // Find the wave resource.
    hResInfo = FindResource (hInst, lpName, TEXT("WAVE")); 
    
    if (hResInfo == NULL) 
      return FALSE; 
    
    // Load the wave resource. 
    hRes = LoadResource (hInst, (HRSRC)hResInfo); 
    
    if (hRes == NULL) 
      return FALSE; 
    
    // Lock the wave resource and play it. 
    lpRes = (LPTSTR)LockResource ((HGLOBAL)hRes);
    
    if (lpRes != NULL) 
      { 
	bRtn = sndPlaySound (lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT ); 
      } 
    else 
      bRtn = 0;
  }
  return bRtn; 
#endif
}

void CreateDirectoryIfAbsent(TCHAR *filename) {
  TCHAR fullname[MAX_PATH];

  LocalPath(fullname, filename);

  DWORD fattr = GetFileAttributes(fullname);

  if ((fattr != 0xFFFFFFFF) &&
      (fattr & FILE_ATTRIBUTE_DIRECTORY)) {
    // directory exists
  } else {
    CreateDirectory(fullname, NULL);
  }

}

bool FileExists(TCHAR *FileName){

  HANDLE hFile = CreateFileW(FileName, GENERIC_READ, 0, NULL,
                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  CloseHandle(hFile);

  return(TRUE);
  
  /*
  FILE *file = _tfopen(FileName, _T("r"));
  if (file != NULL) {
    fclose(file);
    return(TRUE);
  }
  return FALSE;
  */
}

bool RotateScreen() {
#if (WINDOWSPC>0)
  return false;
#else 
  //
  // Change the orientation of the screen
  //
#if 0
  DEVMODE DeviceMode;
    
  memset(&DeviceMode, 0, sizeof(DeviceMode));
  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;
  DeviceMode.dmDisplayOrientation = DMDO_90; 
  //Put your desired position right here.

  if (DISP_CHANGE_SUCCESSFUL == 
      ChangeDisplaySettingsEx(NULL, &DeviceMode, NULL, CDS_RESET, NULL))
    return true;
  else
    return false;
#else
  return false;
#endif
#endif

}


int GetTextWidth(HDC hDC, TCHAR *text) {
  SIZE tsize;
  GetTextExtentPoint(hDC, text, _tcslen(text), &tsize);
  return tsize.cx;
}


void ExtTextOutClip(HDC hDC, int x, int y, TCHAR *text, int width) {
  int len = _tcslen(text);
  if (len <=0 ) {
    return;
  }
  SIZE tsize;
  GetTextExtentPoint(hDC, text, len, &tsize);
  RECT rc;
  rc.left = x;
  rc.top = y;
  rc.right = x + min(width,tsize.cx);
  rc.bottom = y + tsize.cy;

  ExtTextOut(hDC, x, y, /* ETO_OPAQUE | */ ETO_CLIPPED, &rc,
             text, len, NULL);
}

void UpdateConfBB(void) {

  ConfBB[0]=true; // thermal always on automatically
  ConfBB[1]=ConfBB1;
  ConfBB[2]=ConfBB2;
  ConfBB[3]=ConfBB3;
  ConfBB[4]=ConfBB4;
  ConfBB[5]=ConfBB5;
  ConfBB[6]=ConfBB6;
  ConfBB[7]=ConfBB7;
  ConfBB[8]=ConfBB8;
  ConfBB[9]=ConfBB9;

  if (ConfBB2==false && ConfBB3==false &&
      ConfBB4==false && ConfBB5==false &&
      ConfBB6==false && ConfBB7==false &&
      ConfBB8==false && ConfBB9==false)

		// we need at least one bottom bar stripe available (thermal apart)
		ConfBB[1]=true;

}

void UpdateConfIP(void) {

  // MAP MODE always available
  ConfIP[0][0]=true; 
  ConfIP[0][1]=true; 
  ConfMP[0]=true; // map mode

  // LKMODE_INFOMODE is 1
  ConfIP[1][0]=ConfIP11;
  ConfIP[1][1]=ConfIP12;
  ConfIP[1][2]=ConfIP13;
  ConfIP[1][3]=ConfIP14;
  ConfIP[1][4]=ConfIP15;
  ConfIP[1][5]=ConfIP16;

  // WPMODE
  ConfIP[2][0]=ConfIP21;
  ConfIP[2][1]=ConfIP22;
  ConfIP[2][2]=ConfIP23;
  ConfIP[2][3]=ConfIP24;

  // COMMONS
  ConfIP[3][0]=ConfIP31;
  ConfIP[3][1]=ConfIP32;
  ConfIP[3][2]=ConfIP33;

  // TRAFFIC always on if available
  ConfIP[4][0]=true;
  ConfIP[4][1]=true;
  ConfIP[4][2]=true;
  ConfMP[4]=true; // traffic mode

  // Check if we have INFOMODE
  if (ConfIP[1][0]==false && ConfIP[1][1]==false 
	&& ConfIP[1][2]==false && ConfIP[1][3]==false 
	&& ConfIP[1][4]==false && ConfIP[1][5]==false) {
	ConfMP[1]=false;
  } else
	ConfMP[1]=true;

  // Check if we have NEAREST pages
  if (ConfIP[2][0]==false && ConfIP[2][1]==false 
	&& ConfIP[2][2]==false && ConfIP[2][3]==false ) {
	ConfMP[2]=false;
  } else
	ConfMP[2]=true;

  // Check if we have COMMONS
  if (ConfIP[3][0]==false && ConfIP[3][1]==false && ConfIP[3][2]==false ) {
	ConfMP[3]=false;
  } else
	ConfMP[3]=true;

  /*
  // Verify that we have at least one menu
  if (ConfMP[1]==false && ConfMP[2]==false && ConfMP[3]==false ) {
	ConfIP[1][0]=true;
	ConfMP[1]=true;
  }
  */
  SetInitialModeTypes();

}

void SetInitialModeTypes(void) {

  // Update the initial values for each mapspace, keeping the first valid value. We search backwards.
  // INFOMODE 1  
  if (ConfIP[LKMODE_INFOMODE][IM_TRI]) ModeType[LKMODE_INFOMODE]=IM_TRI;
  if (ConfIP[LKMODE_INFOMODE][IM_CONTEST]) ModeType[LKMODE_INFOMODE]=IM_CONTEST;
  if (ConfIP[LKMODE_INFOMODE][IM_AUX]) ModeType[LKMODE_INFOMODE]=IM_AUX;
  if (ConfIP[LKMODE_INFOMODE][IM_TASK]) ModeType[LKMODE_INFOMODE]=IM_TASK;
  if (ConfIP[LKMODE_INFOMODE][IM_THERMAL]) ModeType[LKMODE_INFOMODE]=IM_THERMAL;
  if (ConfIP[LKMODE_INFOMODE][IM_CRUISE]) ModeType[LKMODE_INFOMODE]=IM_CRUISE;

  // WP NEAREST MODE 2  
  if (ConfIP[LKMODE_WP][WP_NEARTPS]) ModeType[LKMODE_WP]=WP_NEARTPS;
  if (ConfIP[LKMODE_WP][WP_LANDABLE]) ModeType[LKMODE_WP]=WP_LANDABLE;
  if (ConfIP[LKMODE_WP][WP_AIRPORTS]) ModeType[LKMODE_WP]=WP_AIRPORTS;

  // COMMONS MODE 3
  if (ConfIP[LKMODE_NAV][NV_HISTORY]) ModeType[LKMODE_WP]=NV_HISTORY;
  if (ConfIP[LKMODE_NAV][NV_COMMONS]) ModeType[LKMODE_WP]=NV_COMMONS;


}


void RestartCommPorts() {

  StartupStore(TEXT(". RestartCommPorts%s"),NEWLINE);

  LockComm();

  devClose(devA());
  devClose(devB());

  NMEAParser::Reset();

  devInit(TEXT(""));

  UnlockComm();

}


void TriggerGPSUpdate()
{
  GpsUpdated = true;
  SetEvent(dataTriggerEvent);
}

// This is currently doing nothing.
void TriggerVarioUpdate()
{
}



bool Debounce(void) {
  static DWORD fpsTimeLast= 0;
  DWORD fpsTimeThis = ::GetTickCount();
  DWORD dT = fpsTimeThis-fpsTimeLast;

  if (dT>(unsigned int)debounceTimeout) {
    fpsTimeLast = fpsTimeThis;
    return true;
  } else {
    return false;
  }
}

//
// Let's get rid of BOOOOls soon!!!
bool BOOL2bool(BOOL a) {
  if (a==TRUE) return true;
  return false;
}

