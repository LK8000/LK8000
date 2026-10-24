/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Polar.h
 * Author: Bruno de Lacheisserie
 */
#pragma once
#include "WinPilot.h"
#include "Quadratic.h"
#include <string_view>

void WeightOffset(double wload);

bool ReadWinPilotPolar();


namespace GliderPolar {

bool Update(const Quadratic& polar, double DryWeight, double BallastWeight);

bool Update(const Quadratic& polar);

bool Update(double sproj, double ar, double auw,
            const std::string_view& harness);

} // namespace GliderPolar
