/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: Radio.h,v 1.1 2020/22/12 
 */

#include "options.h"
#include "externs.h"
#include "Radio.h"
#include "Util/TruncateString.hpp"


bool ValidFrequency(unsigned khz) {

	static constexpr unsigned _25_[] = {
		 0, 25, 50, 75
	};

	static constexpr unsigned _8_33_[] = { 
		 5, 10, 15, 30, 35, 40,
		55, 60, 65, 80, 85, 90
	};

	if (khz >= 118000 && khz <= 137000) {

		unsigned sub = khz - ((khz / 100) * 100);

		for (auto d : _25_) {
			if (d == sub) {
				return true;
			}
		}

		if (RadioPara.Enabled8_33) {
			for (auto d : _8_33_) {
				if (d == sub) {
					return true;
				}
			}
		}
	}
	return false;
}

unsigned ExtractFrequency(const TCHAR *text, size_t* start, size_t* len) {
	if (text == nullptr) {
		return 0;
	}

	for (const TCHAR* c = text; *c; ++c) {
		if (*c == '1') {
			TCHAR* dot = nullptr;
			unsigned khz = _tcstol(c, &dot, 10);
			if (khz >= 118 && khz <= 138) {
				if ((*dot == _T('.')) || (*dot == _T(','))) {
					++dot;
					while (khz < 100000 && isdigit((*dot))) {
						khz = (khz * 10) + (*dot) - '0';
						++dot;
					}
				}
			}

			while (khz < 100000) {
				khz *= 10;
			}

			if(ValidFrequency(khz)) {
				if (start) {
					*start = std::distance<const TCHAR*>(text, c);
				}
				if (len) {
					*len = std::distance<const TCHAR*>(c, dot);
				}
				return khz;
			}
			c = dot;
		}
	}

	return 0;
}

/**
 * Distance in ° between two geographical position
 * 
 * using it to compare tp distance is an aproximation and  is valid 
 *   only to fast solve conflict between to TP with same frequency 
 * 
 * if needed to improve precision dlat calculation can be replaced by
 *   double dlat =  cos(a.latitude * PI/180) * (a.latitude - b.latitude)
 */
static double LatLonDistance(const GeoPoint& a, const GeoPoint& b)
{
	double dla = a.latitude - b.latitude;
	double dlo = a.longitude - b.longitude;
	return sqrt(dla*dla + dlo*dlo);
}


bool UpdateStationName(TCHAR (&Name)[NAME_SIZE + 1], unsigned khz) {

	double minDist = 9999999;
	int idx = -1;
	
	if(!ValidFrequency(khz))
		return 0;

	GeoPoint cur_pos = WithLock(CritSec_FlightData, GetCurrentPosition, GPS_INFO);

	LockTaskData();
	for (size_t i = NUMRESWP; i < WayPointList.size(); ++i) {
		const WAYPOINT& wpt = WayPointList[i];

		assert(wpt.Latitude != RESWP_INVALIDNUMBER);

		if (wpt.Freq[0]) { // ignore TP with empty frequency

			unsigned WpFreq = ExtractFrequency(wpt.Freq);
			if (khz  == WpFreq) {
				double fDist = LatLonDistance(cur_pos, GeoPoint(wpt.Latitude, wpt.Longitude));
				if(fDist < minDist) {
					minDist = fDist;
					idx = i;
				}
			}
		}
	}

	_tcscpy(Name, (idx >= 0) ? WayPointList[idx].Name : _T(""));

	UnlockTaskData();		

	return (idx >= 0);
}


static int SearchNearestStation(GeoPoint cur_pos) {
	double minDist = 9999999;
	int minIdx = -1;

	for (size_t i = NUMRESWP; i < WayPointList.size(); ++i)
	{
		const WAYPOINT& wpt = WayPointList[i];

		assert(wpt.Latitude != RESWP_INVALIDNUMBER);

		if(wpt.Freq[0]) { // ignore TP with empty frequency
			double fWpFreq = StrToDouble(wpt.Freq, nullptr);
			if(ValidFrequency( fWpFreq))
			{					
				double fDist = LatLonDistance(cur_pos, GeoPoint(wpt.Latitude, wpt.Longitude));
				if(fDist < minDist)
				{
					minDist = fDist;
					minIdx =i;
				}
			}
		}
	}

	return minIdx;
}

std::optional<RadioStation> SearchBestStation(const GeoPoint& cur_pos) {

	ScopeLock lock(CritSec_TaskData);

	int Idx = BestAlternate;    // begin with Best alternate
	unsigned khz = 0;

	if(ValidWayPointFast(Idx)) {
		khz = ExtractFrequency(WayPointList[Idx].Freq);
	}

	if(!ValidFrequency(khz)) { 
		// best alternate does not have a radio?
		Idx = SearchNearestStation(cur_pos); // OK, then search for the nearest with radio!
	}

	if(ValidWayPointFast(Idx)) {
		// found, return station frequency and name
		return RadioStation{
			ExtractFrequency(WayPointList[Idx].Freq),
			WayPointList[Idx].Name
		};
	}

	return {};
}

const TCHAR* GetActiveStationSymbol(bool unicode_symbol) {
	if (!unicode_symbol) {
		return _T("X");
	}
	// Up Down Arrow "↕"
#ifdef UNICODE
	return L"\u2195"; // utf-16 (WIN32)
#else
	return "\xE2\x86\x95"; // utf-8
#endif
}

const TCHAR* GetStandyStationSymbol(bool unicode_symbol) {
	if (!unicode_symbol) {
		return _T("v");
	}
	// Downwards Arrow "↓"
#ifdef UNICODE
	return L"\u2193"; // utf-16 (WIN32)
#else
	return "\xE2\x86\x93"; // utf-8
#endif
}

void AutomaticRadioStation(const GeoPoint& cur_pos) {

	if (!RadioPara.Enabled) {
		return;
	}

	// auto frequency selection?
	if (bAutoActive || bAutoPassiv) {

		auto optional = SearchBestStation(cur_pos);
		if (optional) {
			auto& station = optional.value();

			if (bAutoActive) {
				if (devPutFreqActive(station.Khz, station.name.c_str())) {
					RadioPara.Changed = true;
				}
			}

			if (bAutoPassiv) {
				if (devPutFreqStandby(station.Khz, station.name.c_str())) {
					RadioPara.Changed = true;
				}
			}
		}
	}
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_SUITE("Radio") {

	TEST_CASE("ValidFrequency") {

		SUBCASE("25kHz radio") {
			RadioPara.Enabled8_33 = false;

			CHECK_FALSE(ValidFrequency(110000));
			CHECK_FALSE(ValidFrequency(140000));
			CHECK_FALSE(ValidFrequency(118005));
			CHECK_FALSE(ValidFrequency(118004));
			CHECK_FALSE(ValidFrequency(140000));

			for (unsigned f = 118000; f < 137000; f += 25) {
				CAPTURE(f);
				CHECK(ValidFrequency(f));
			}
		}

		SUBCASE("8.33kHz radio") {
			RadioPara.Enabled8_33 = true;
			for (unsigned f = 118000; f < 137000; f += 100) {
				CAPTURE(f);

				CHECK(ValidFrequency(f + 05));
				CHECK(ValidFrequency(f + 10));
				CHECK(ValidFrequency(f + 15));

				CHECK(ValidFrequency(f + 30));
				CHECK(ValidFrequency(f + 35));
				CHECK(ValidFrequency(f + 40));

				CHECK(ValidFrequency(f + 55));
				CHECK(ValidFrequency(f + 60));
				CHECK(ValidFrequency(f + 65));

				CHECK(ValidFrequency(f + 80));
				CHECK(ValidFrequency(f + 85));
				CHECK(ValidFrequency(f + 90));
			}
		}
	}

	TEST_CASE("ExtractFrequency") {

		CHECK_EQ(ExtractFrequency(_T("118")), 118000U);
		CHECK_EQ(ExtractFrequency(_T("118025")), 118025U);
		CHECK_EQ(ExtractFrequency(_T("118.025")), 118025U);
		CHECK_EQ(ExtractFrequency(_T("118,025")), 118025U);
		CHECK_EQ(ExtractFrequency(_T("1185")), 118500U);
		CHECK_EQ(ExtractFrequency(_T("11850")), 118500U);
		CHECK_EQ(ExtractFrequency(_T("118500")), 118500U);
		CHECK_EQ(ExtractFrequency(_T("118.5")), 118500U);
		CHECK_EQ(ExtractFrequency(_T("118.50")), 118500U);

		size_t start, len;

		CHECK_EQ(ExtractFrequency(_T("118.500"), &start, &len), 118500U);
		CHECK_EQ(start, 0);
		CHECK_EQ(len, 7);

		CHECK_EQ(ExtractFrequency(_T("azazaz 118025 azazaz "), &start, &len), 118025U);
		CHECK_EQ(start, 7);
		CHECK_EQ(len, 6);

		CHECK_EQ(ExtractFrequency(_T("azazaz 118.025 azazaz "), &start, &len), 118025U);
		CHECK_EQ(start, 7);
		CHECK_EQ(len, 7);

		CHECK_EQ(ExtractFrequency(_T("azazaz 118,025 azazaz "), &start, &len), 118025U);
		CHECK_EQ(start, 7);
		CHECK_EQ(len, 7);
	}
}

#endif
