/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#include <memory.h>

#include "externs.h"


#define EGM96_H 90
#define EGM96_W 180
#define EGM96SIZE EGM96_W * EGM96_H //16200



#ifdef WIN32_RESOURCE
extern HINSTANCE _hInstance;
#else
#include "resource_data.h"
#endif
const unsigned char* egm96data = nullptr;

void OpenGeoid() {
#ifdef WIN32_RESOURCE
        LKASSERT(!egm96data);
        egm96data = nullptr;
        HRSRC hResInfo = FindResource(_hInstance, TEXT("IDR_RASTER_EGM96S"), TEXT("RASTERDATA"));
        if (hResInfo) {
            HGLOBAL hRes = LoadResource(_hInstance, hResInfo);
            if (hRes) {
                // Retrieves a pointer to the resource in memory
                const unsigned char* lpRes = static_cast<const unsigned char*>(LockResource(hRes));
                if (lpRes) {
                    const size_t len = SizeofResource(_hInstance, hResInfo);
                    if (len == EGM96SIZE) {
                        egm96data = lpRes;
                    } else {
                        StartupStore(_T(". EGM96SIZE error%s"),NEWLINE);
                    }
                }
            } else {
                StartupStore(_T(". UNABLE TO LOAD RESOURCE FOR OPENGEOID%s"),NEWLINE);
            }
        } else {
            StartupStore(_T(". UNABLE TO FIND RESOURCE FOR OPENGEOID%s"),NEWLINE);
        }
        if (!egm96data) StartupStore(_T(". OPENGEOID FAILURE!%s"),NEWLINE);
#else
        if(std::distance(IDR_RASTER_EGM96S_begin, IDR_RASTER_EGM96S_end) == EGM96SIZE) {
            egm96data = IDR_RASTER_EGM96S_begin;
        }
#endif
        LKASSERT(egm96data);
}

void CloseGeoid() {
    egm96data = nullptr;
}

static inline 
double getEGM96data(int x, int y) {
    return (double)(egm96data[x+y*EGM96_W])-127;
}


static inline 
double interpolation2d(double x, double y, double z11, double z12, double z21, double z22) {
    return (z22*y*x+z12*(1-y)*x+z21*y*(1-x)+z11*(1-y)*(1-x)); //x and y must be between 0 and 1
}


double LookupGeoidSeparation(double lat, double lon) {
  BUGSTOP_LKASSERT(egm96data); // OpenGeoid must be called before. 
  if (!egm96data) {
      return 0.0;
  }

  double y=(90.0-lat)/2.0;
  int ilat=(int)y;
  if(lon<0) lon+= 360.0;
  double x=lon/2.0;
  int ilon=(int)x;
  if(ilat>EGM96_H || ilon>EGM96_W || ilat<0 || ilon<0) return 0.0;
  if(ilat==EGM96_H || ilat==EGM96_H-1) return getEGM96data(ilon,EGM96_H-1); //to prevent to go over -90
  int ilonp1;
  if(ilon!=EGM96_W-1) ilonp1=ilon+1;
  else ilonp1=0; //in this case interpolate through the Greenwich meridian
  x-=(double)ilon;
  y-=(double)ilat;
  return interpolation2d(x,y,getEGM96data(ilon,ilat),getEGM96data(ilonp1,ilat),getEGM96data(ilon,ilat+1),getEGM96data(ilonp1,ilat+1));
}
