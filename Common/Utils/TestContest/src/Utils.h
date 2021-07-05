/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.h,v 8.3 2010/12/16 14:44:47 root Exp root $
*/

#if !defined(AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_UTILS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include <windows.h>
#include <math.h>

double AngleLimit360(double theta);

void DistanceBearing(double lat1, double lon1, double lat2, double lon2,
                     double *Distance, double *Bearing);

#endif
