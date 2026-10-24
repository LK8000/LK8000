/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "GliderPolar/Polar.h"
#include "McReady.h"
#include "Dialogs.h"
#include "utils/zzip_file_stream.h"
#include "utils/charset_helper.h"
#include "LocalPath.h"

// This is calculating the weight difference for the chosen wingloading
// Remember that polar files have a weight indicated that includes the pilot..
// Check that WingArea is NOT zero! Winpilot polars had no WingArea configurable!

void WeightOffset(double wload) {
  // we should lock calculation thread probably

  if (GlidePolar::WingArea < 1) {  // 100131
    GlidePolar::WeightOffset = 0;
    return;
  }

  // WEIGHTS[WEIGHT_WATER] is full ballast
  // BALLAST is percentage of full ballast
  // new weight = (wingload * wingarea) - ballast
  double calcweight =
      (wload * GlidePolar::WingArea) - (WEIGHTS[WEIGHT_WATER] * BALLAST);

  // We set a min limit here, see SetBallast()

  // use 1.0 kg/m2 for minimum wingloading, since wingloading range each type of
  // gliders are different, and we want to avoid negative weight offset.
  //
  //   Paraglider     3–8 kg/m²
  //   Hang glider    5–12 kg/m²
  //   Sailplane      30–65+ kg/m²

  constexpr double min_wload = 1.0;

  const double dry_gross_weight = WEIGHTS[WEIGHT_PILOT] + WEIGHTS[WEIGHT_PLANEDRY];
  const double min_offset = ((min_wload * GlidePolar::WingArea) - dry_gross_weight); 

  GlidePolar::WeightOffset = std::max(calcweight - dry_gross_weight, min_offset);

  GlidePolar::SetBallast(); // BUGFIX 101002
}

static bool PolarWinPilot2XCSoar(double (&dPOLARV)[3], double (&dPOLARW)[3],
                                 double (&ww)[2], double WingArea) {

  auto kmh_to_ms = [](double v) { return Units::From(Units_t::unKiloMeterPerHour, v); };

  GliderPolar::Quadratic quadratic({kmh_to_ms(dPOLARV[0]), dPOLARW[0]},
                                   {kmh_to_ms(dPOLARV[1]), dPOLARW[1]},
                                   {kmh_to_ms(dPOLARV[2]), dPOLARW[2]});

  if (GliderPolar::Update(quadratic, ww[0], ww[1])) {
    GlidePolar::WingArea = WingArea;
    return true;
  }

  return false;
}

bool ReadWinPilotPolar(void) {

  TCHAR	szFile[MAX_PATH] = TEXT("\0");
  TCHAR ctemp[80];

  double dPOLARV[3];
  double dPOLARW[3];
  double ww[2];
  double WingArea;
  bool foundline = false;

  // STD.CIRRUS values, overwritten by loaded values
  // MassDryGross[kg], MaxWaterBallast[liters], Speed1[km/h], Sink1[m/s], Speed2, Sink2, Speed3, Sink3
  // 337, 80, 93.23, -0.74, 149.17, -1.71, 205.1, -4.2, 10.04
  ww[0]= 337;
  ww[1]= 80;
  dPOLARV[0]= 93.23;
  dPOLARW[0]= -0.74;
  dPOLARV[1]= 149.17;
  dPOLARW[1]= -1.71;
  dPOLARV[2]= 205.1;
  dPOLARW[2]= -4.2;

  GlidePolar::WeightOffset=0;

    if (_tcscmp(szPolarFile,_T(""))==0) {
        StartupStore(_T("... Empty polar file, using Default" NEWLINE));
        lk::strcpy(szPolarFile,_T(LKD_DEFAULT_POLAR));
    }

    /**
     * szPolarFile can be :
     *   1 - absolute path
     *   2 - relative path to external directory ( LocalPath )
     *   3 - retlative path to external directory but with old filename ( migration from V5 or older )
     *   4 - relative path to system directory
     *
     * it's important to try in this order.
     * in worst case we try to open file 4 time, but timing is not important here.
     */

    tstring str (szPolarFile);
    lk::strcpy(szFile, str.c_str());
    zzip_file_stream stream(szFile, "rt");
    if(!stream) {
        // failed to open absolute. try LocalPath
        LocalPath(szFile, _T(LKD_POLARS), str.c_str());
        stream = zzip_file_stream(szFile, "rt");
    }
    if(!stream){
        // failed to open Local. try with converted file name to new file name.
        // polar file name can be an old name, convert to new name and retry.
        bool bRetry = false;
        const TCHAR strReplace[] = _T(" ()");
        for(std::size_t found = str.find_first_of(strReplace); found!=std::string::npos; found=str.find_first_of(strReplace,found+1)) {
          str[found]=_T('_');
          bRetry = true; // only retry if file name change.
        }

        if(bRetry) {
            LocalPath(szFile,_T(LKD_POLARS), str.c_str());
            stream = zzip_file_stream(szFile, "rt");
        }
    }
    if(!stream) {
        // all previous failed. try SystemPath
        SystemPath(szFile, _T(LKD_SYS_POLAR), str.c_str());
        stream = zzip_file_stream(szFile, "rt");
    }

    StartupStore(_T(". Loading polar file <%s>%s"),szFile,NEWLINE);
    if (stream) {
      std::istream in(&stream);
      std::string src_line;

      while (std::getline(in, src_line) && (!foundline)) {
        if (src_line.size() < 10) {
          continue;
        }
        if (src_line.starts_with("*")) { // Look For Comment
          continue;
        }
        tstring String = from_unknown_charset(src_line.c_str());
        PExtractParameter(String.c_str(), ctemp, 0);
        // weight of glider + pilot
        ww[0] = StrToDouble(ctemp, NULL);
        // weight of loadable ballast
        PExtractParameter(String.c_str(), ctemp, 1);
        ww[1] = StrToDouble(ctemp, NULL);

        PExtractParameter(String.c_str(), ctemp, 2);
        dPOLARV[0] = StrToDouble(ctemp, NULL);
        PExtractParameter(String.c_str(), ctemp, 3);
        dPOLARW[0] = StrToDouble(ctemp, NULL);

        PExtractParameter(String.c_str(), ctemp, 4);
        dPOLARV[1] = StrToDouble(ctemp, NULL);
        PExtractParameter(String.c_str(), ctemp, 5);
        dPOLARW[1] = StrToDouble(ctemp, NULL);

        PExtractParameter(String.c_str(), ctemp, 6);
        dPOLARV[2] = StrToDouble(ctemp, NULL);
        PExtractParameter(String.c_str(), ctemp, 7);
        dPOLARW[2] = StrToDouble(ctemp, NULL);

        ctemp[0] = _T('\0');
        PExtractParameter(String.c_str(), ctemp, 8);
        if (_tcscmp(ctemp, _T("")) != 0) {
          WingArea = StrToDouble(ctemp, NULL);
        }
        else {
          WingArea = 0.0;
        }

        TestLog(
            _T("... Polar ww0=%.2f ww1=%.2f v0=%.2f,%.2f v1=%.2f,%f ")
            _T("v2=%.2f,%.2f area=%.2f"),
            ww[0], ww[1], dPOLARV[0], dPOLARW[0], dPOLARV[1], dPOLARW[1],
            dPOLARV[2], dPOLARW[2], WingArea);

        if (ww[0] <= 0 || dPOLARV[0] == 0 || dPOLARW[0] == 0 ||
            dPOLARV[1] == 0 || dPOLARW[1] == 0 || dPOLARV[2] == 0 ||
            dPOLARW[2] == 0) {
          continue;  // read another line searching for polar
        }
        else {
          if (WingArea == 0) {
            StartupStore(_T("... WARNING Polar file has NO wing area"));
          }
          foundline = PolarWinPilot2XCSoar(dPOLARV, dPOLARW, ww, WingArea);
        }
      }

      // Reset flaps values after loading a new polar, and init FlapsPos for the
      // first time
      for (int i = 0; i < MAX_FLAPS; i++) {
        GlidePolar::FlapsPos[i] = 0.0;
        lk::strcpy(GlidePolar::FlapsName[i], _T("???"));
      }
      GlidePolar::FlapsPosCount = 0;
      GlidePolar::FlapsMass = 0.0;

      // Unless we check valid string, even with empty string currentFlapsPos
      // will be positive, and thus force Flaps calculations even with no
      // extended polar. Let's allow empty lines and comments in the polar file,
      // before the flaps line is found.
      //
      do {
        if (src_line.size() < 10) {
          continue;
        }
        if (src_line.starts_with("*")) {
          continue;
        }
        tstring String = from_unknown_charset(src_line.c_str());
        // try to read flaps configuration line
        PExtractParameter(String.c_str(), ctemp, 0);
        GlidePolar::FlapsMass = StrToDouble(ctemp, NULL);
        PExtractParameter(String.c_str(), ctemp, 1);
        int flapsCount = (int)StrToDouble(ctemp, NULL);

        // int currentFlapsPos = 0;
        // GlidePolar::FlapsPos[currentFlapsPos][0] = 0.0;  // no need, already
        // initialised

        int currentFlapsPos = 1;
        for (int i = 2; i <= flapsCount * 2; i = i + 2) {
          PExtractParameter(String.c_str(), ctemp, i);
          GlidePolar::FlapsPos[currentFlapsPos] = StrToDouble(ctemp, NULL);
          if (GlidePolar::FlapsPos[currentFlapsPos] > 0) {
            GlidePolar::FlapsPos[currentFlapsPos] = Units::From(
                unKiloMeterPerHour, GlidePolar::FlapsPos[currentFlapsPos]);
          }
          PExtractParameter(String.c_str(), ctemp, i + 1);
          ctemp[MAXFLAPSNAME] = '\0';
          size_t len = _tcslen(ctemp);
          if (len > 0 && (ctemp[_tcslen(ctemp) - 1] == '\r' ||
                          ctemp[_tcslen(ctemp) - 1] == '\n')) {
            ctemp[_tcslen(ctemp) - 1] = '\0';  // remove trailing cr
          }
          lk::strcpy(GlidePolar::FlapsName[currentFlapsPos], ctemp);
          if (currentFlapsPos >= (MAX_FLAPS - 1))
            break;  // safe check
          currentFlapsPos++;
        }
        lk::strcpy(GlidePolar::FlapsName[0], GlidePolar::FlapsName[1]);
        GlidePolar::FlapsPos[currentFlapsPos] = MAXSPEED;
        lk::strcpy(GlidePolar::FlapsName[currentFlapsPos], ctemp);
        currentFlapsPos++;
        GlidePolar::FlapsPosCount = currentFlapsPos;
        break;
      } while (std::getline(in, src_line));
  } else {
        StartupStore(_T("... Polar file <%s> not found!%s"),szFile,NEWLINE);
  }

	if (!foundline) {
		StartupStore(_T("... INVALID POLAR FILE! POLAR RESET TO DEFAULT: Std.Cirrus\n"));
		ww[0]= 337;
		ww[1]= 80;
		dPOLARV[0]= 93.23;
		dPOLARW[0]= -0.74;
		dPOLARV[1]= 149.17;
		dPOLARW[1]= -1.71;
		dPOLARV[2]= 205.1;
		dPOLARW[2]= -4.2;
		WingArea = 10.04;
		gcc_unused bool bok = PolarWinPilot2XCSoar(dPOLARV, dPOLARW, ww, WingArea);
		assert(bok);
		lk::strcpy(szPolarFile,_T(LKD_DEFAULT_POLAR));

		MessageBoxX(MsgToken<920>(), // Error loading Polar file!
								MsgToken<791>(), // Warning
								mbOk);
	} // !foundline

	GlidePolar::SetBallast();

	return(foundline);
}
