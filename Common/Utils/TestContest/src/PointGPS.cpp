/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "PointGPS.h"
#include "Tools.h"
#include <ostream>


std::ostream &operator<<(std::ostream &stream, const CPointGPS &point)
{
  stream << "Time:      " << TimeToString(point._time) << std::endl;
  stream << "Latitude:  " << CoordToString(point._lat, true) << std::endl;
  stream << "Longitude: " << CoordToString(point._lon, false) << std::endl;
  stream << "Altitude:  " << static_cast<unsigned>(point._alt) << "m" << std::endl;
  return stream;
}
