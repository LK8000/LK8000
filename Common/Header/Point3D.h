/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Point3D.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __POINT3D_H__
#define __POINT3D_H__

#include "Point2D.h"


/** 
 * @brief 3D Point data
 * 
 * CPoint3D class stores the location of a geo point. It also provides
 * basic operations on those points.
 */
class CPoint3D : public CPoint2D {
  int _alt;
public:
  CPoint3D(double lat, double lon, int alt):
    CPoint2D(lat, lon), _alt(alt) {}
  int Altitude() const { return _alt; }
};

typedef std::shared_ptr<const CPoint3D> CPoint3DSmart;
typedef std::vector<CPoint3D> CPoint3DArray;


#endif /* __POINT3D_H__ */
