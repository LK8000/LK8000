/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "McReady.h"
#include "Calc/Vario.h"

void NettoVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  if (!Calculated->Flying) {
    Calculated->NettoVario = 0.0;
    return;
  }

  // get load factor
  const double GLoad = Calculated->Gload;

  // calculate sink rate of glider for calculating netto vario
  const bool replay_disabled = !ReplayLogger::IsEnabled();

  const double ias = (Basic->AirspeedAvailable && replay_disabled)
          ? Basic->IndicatedAirspeed
          : Calculated->IndicatedAirspeedEstimated;

  const double glider_sink_rate = AirDensitySinkRate(std::max(GlidePolar::Vminsink(), ias), Basic->Altitude, GLoad);

  if (Basic->NettoVarioAvailable && replay_disabled) {
    Calculated->NettoVario = Basic->NettoVario;
  }
  else if (VarioAvailable(*Basic) && replay_disabled) {
    Calculated->NettoVario = Basic->Vario - glider_sink_rate;
  }
  else {
    Calculated->NettoVario = Calculated->Vario - glider_sink_rate;
  }
}
