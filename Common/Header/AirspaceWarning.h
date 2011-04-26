/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2008  

  	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  $Id: AirspaceWarning.h,v 1.2 2009/06/08 13:54:43 jwharington Exp $
}
*/
#ifndef AIRSPACE_WARNING_H
#define AIRSPACE_WARNING_H

#include "Calculations.h"

#ifdef LKAIRSPACE
extern int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, const CAirspace *airspace,
                                bool ackDay=false);
#else
extern void AirspaceWarnListAdd(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, bool IsCircle, int AsIdx,
                                bool ackDay=false);

extern int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                                bool Predicted, bool IsCircle, int AsIdx,
                                bool ackDay=false);

extern void AirspaceWarnListProcess(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
#endif


#endif
