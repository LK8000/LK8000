/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LDRotaryBuffer.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "LDRotaryBuffer.h"
#include "utils/lookup_table.h"

LDRotary rotaryLD;

double LDRotary::GetPeriod() {
	constexpr auto table = lookup_table<AverEffTime_t, double>({
		{ ae3seconds, 3 },
		{ ae5seconds, 5 },
		{ ae10seconds, 10 },
		{ ae15seconds, 15 },
		{ ae30seconds, 30 },
		{ ae45seconds, 45 },
		{ ae60seconds, 60 },
		{ ae90seconds, 90 },
		{ ae2minutes, 120 },
		{ ae3minutes, 180 }
	});
	return table.get(static_cast<AverEffTime_t>(AverEffTime), 3);
}

void LDRotary::Init() {
	buffer.init(GetPeriod());
	last_altitude.reset();
}

void LDRotary::Insert(double distance, const NMEA_INFO& Basic, DERIVED_INFO& Calculated) {
	if (Calculated.OnGround) {
		return;
	}
	if (!ISCAR && Calculated.Circling) {
		return;
	}

	if (last_altitude) {
		buffer.insert(Basic.Time, {
			distance,
			(*last_altitude) - Calculated.NavAltitude,
			Basic.AirspeedAvailable ? Basic.IndicatedAirspeed : Calculated.IndicatedAirspeedEstimated,
			Basic.Speed
		});
	}
	last_altitude = Calculated.NavAltitude;
}

void LDRotary::Calculate(const NMEA_INFO& Basic, DERIVED_INFO& Calculated) {
	if ( buffer.empty() || Calculated.Circling || Calculated.OnGround || !Calculated.Flying ) {
		Calculated.EqMc = -1;
		Calculated.AverageLD = 0;
		Calculated.AverageGS = 0;
		Calculated.AverageDistance = 0;
		return;
	}
	auto average = buffer.average();
	if (!average) {
		return;
	}

	Calculated.EqMc = GlidePolar::EquMC(average->ias);
	// Do not consider impossible MC values as Equivalent
	if (Calculated.EqMc < 0 || Calculated.EqMc > 20) {
		Calculated.EqMc = -1;
	}


	Calculated.AverageGS = average->speed;
	Calculated.AverageDistance = average->distance;

	const auto& sum = buffer.sum();

	Calculated.AverageLD = (sum.altitude > 0.)
								? sum.distance / sum.altitude
								: INVALID_GR;

	if (Calculated.AverageLD > MAXEFFICIENCYSHOW) {
		Calculated.AverageLD = INVALID_GR;
	}
}
