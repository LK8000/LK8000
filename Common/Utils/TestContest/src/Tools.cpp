/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "Tools.h"
#include <cmath>
#include <iomanip>


std::string TimeToString(unsigned time)
{
  unsigned hours = time / 3600;
  unsigned minutes = (time - hours * 3600) / 60;
  unsigned seconds = time - hours * 3600 - minutes * 60;
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds;
  return stream.str();
}


std::string CoordToString(double coord, bool latitude)
{
  std::stringstream stream;
  stream << std::fixed << std::setfill('0') << std::setw(latitude ? 2 : 3) << abs(coord) << " " << std::setw(2) << std::setprecision(3) << fabs(coord - (int)coord) * 60;
  if(latitude)
    stream << (coord > 0 ? "N" : "S");
  else
    stream << (coord > 0 ? "E" : "W");
  return stream.str();
}
