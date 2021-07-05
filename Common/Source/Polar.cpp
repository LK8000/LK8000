/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "utils/openzip.h"
#include "utils/zzip_stream.h"
#include "Utils.h"

// This is calculating the weight difference for the chosen wingloading
// Remember that polar files have a weight indicated that includes the pilot..
// Check that WingArea is NOT zero! Winpilot polars had no WingArea configurable!

void WeightOffset(double wload) {
  double calcweight;

  if (GlidePolar::WingArea<1) { // 100131
	// we should lock calculation thread probably
	if (GlidePolar::WeightOffset!=0) GlidePolar::WeightOffset=0;
	return;
  }

  // WEIGHTS[2] is full ballast
  // BALLAST is percentage of full ballast
  // new weight = (wingload * wingarea) - ballast
  calcweight=(wload*GlidePolar::WingArea) - (WEIGHTS[2]*BALLAST);
  // We set a min limit here, see SetBallast()
  // Probably only UAV can have such low wing loadings
  // Or a gnome on an RC glider, maybe.
  GlidePolar::WeightOffset = std::max(calcweight-WEIGHTS[0]-WEIGHTS[1],(GlidePolar::WingArea - WEIGHTS[0] - WEIGHTS[1]));

  GlidePolar::SetBallast(); // BUGFIX 101002
}


bool PolarWinPilot2XCSoar(double (&dPOLARV)[3], double (&dPOLARW)[3], double (&ww)[2]) {

  POLARV[0] = dPOLARV[0];
  POLARV[1] = dPOLARV[1];
  POLARV[2] = dPOLARV[2];
  POLARLD[0] = dPOLARW[0];
  POLARLD[1] = dPOLARW[1];
  POLARLD[2] = dPOLARW[2];
  WW[0]  = ww[0];
  WW[1]  = ww[1];

  const double v1 = dPOLARV[0]/3.6; 
  const double v2 = dPOLARV[1]/3.6; 
  const double v3 = dPOLARV[2]/3.6;
  
  const double w1 = dPOLARW[0]; 
  const double w2 = dPOLARW[1]; 
  const double w3 = dPOLARW[2];

  double d = v1 * v1 * (v2 - v3) + v2 * v2 * (v3 - v1) + v3 * v3 * (v1 - v2);
  if (d == 0.0) {
    POLAR[0] = 0.;
  } else {
    POLAR[0] = ((v2 - v3)*(w1 - w3)+(v3 - v1)*(w2 - w3)) / d;
  }
  d = v2 - v3;
  if (d == 0.0) {
    POLAR[1] = 0.;
  } else {
    POLAR[1] = (w2 - w3 - POLAR[0]*(v2 * v2 - v3 * v3)) / d;
  }
  POLAR[2] = (w3 - POLAR[0] *v3*v3 - POLAR[1]*v3);
  
  // check polar validity : 
  if(POLAR[0] > 0.) {
    // "a"  must be negative
    return false;
  }
  
  const double x = -1. * (POLAR[1]/ (2 *  POLAR[0]));
  if( x < 0 )  {
    // minsink speed must be positive
    return false;
  }
  
  const double y = POLAR[0] *x*x + POLAR[1]*x + POLAR[2];
  if(y > 0.) {
    // minsink must be negative
    return false;
  }
  
  // these 0 and 1 are always used as a single weight: always 0+1 everywhere
  // If WEIGHT 0 is used also WEIGHT 1 is used together, so it is unnecessary to keep both values.
  // however it doesnt hurt .
  // For this reason, the 70kg pilot weight is not important.
  // If we want to adjust wingloading, we just need to change gross weight.
  WEIGHTS[0] = 70;                      // Pilot weight
  WEIGHTS[1] = ww[0]-WEIGHTS[0];        // Glider empty weight
  WEIGHTS[2] = ww[1];                   // Ballast weight


  // now scale off weight
  BUGSTOP_LKASSERT((WEIGHTS[0] + WEIGHTS[1])>=0);
  if((WEIGHTS[0] + WEIGHTS[1])>=0) {
    POLAR[0] = POLAR[0] * (double)sqrt(WEIGHTS[0] + WEIGHTS[1]);
    POLAR[2] = POLAR[2] / (double)sqrt(WEIGHTS[0] + WEIGHTS[1]);
  }
  
  return true;
}



bool ReadWinPilotPolar(void) {

  TCHAR	szFile[MAX_PATH] = TEXT("\0");
  TCHAR ctemp[80];
  TCHAR TempString[READLINE_LENGTH+1];

  double dPOLARV[3];
  double dPOLARW[3];
  double ww[2];
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
        _tcscpy(szPolarFile,_T(LKD_DEFAULT_POLAR));
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
    _tcscpy(szFile, str.c_str());
    zzip_stream stream(szFile, "rt");
    if(!stream) {
        // failed to open absolute. try LocalPath
        LocalPath(szFile, _T(LKD_POLARS), str.c_str());
        stream.open(szFile, "rt");
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
            stream.open(szFile, "rt");
        }
    }
    if(!stream) {
        // all previous failed. try SystemPath
        SystemPath(szFile, _T(LKD_SYS_POLAR), str.c_str());
        stream.open(szFile, "rt");
    }

    StartupStore(_T(". Loading polar file <%s>%s"),szFile,NEWLINE);
    if (stream){

        while(stream.read_line(TempString) && (!foundline)){

		if (_tcslen(TempString) <10) continue;

          if(_tcsstr(TempString,TEXT("*")) != TempString) // Look For Comment
            {
              PExtractParameter(TempString, ctemp, 0);
		// weight of glider + pilot
              ww[0] = StrToDouble(ctemp,NULL);
		// weight of loadable ballast
              PExtractParameter(TempString, ctemp, 1);
              ww[1] = StrToDouble(ctemp,NULL);

              PExtractParameter(TempString, ctemp, 2);
              dPOLARV[0] = StrToDouble(ctemp,NULL);
              PExtractParameter(TempString, ctemp, 3);
              dPOLARW[0] = StrToDouble(ctemp,NULL);

              PExtractParameter(TempString, ctemp, 4);
              dPOLARV[1] = StrToDouble(ctemp,NULL);
              PExtractParameter(TempString, ctemp, 5);
              dPOLARW[1] = StrToDouble(ctemp,NULL);

              PExtractParameter(TempString, ctemp, 6);
              dPOLARV[2] = StrToDouble(ctemp,NULL);
              PExtractParameter(TempString, ctemp, 7);
              dPOLARW[2] = StrToDouble(ctemp,NULL);

              ctemp[0] = _T('\0');
              PExtractParameter(TempString, ctemp, 8);
		if ( _tcscmp(ctemp,_T("")) != 0) {
			GlidePolar::WingArea = StrToDouble(ctemp,NULL);
		} else {
			GlidePolar::WingArea = 0.0;
		}

              #if TESTBENCH
              StartupStore(_T("... Polar ww0=%.2f ww1=%.2f v0=%.2f,%.2f v1=%.2f,%f v2=%.2f,%.2f area=%.2f\n"),
                  ww[0], ww[1], dPOLARV[0], dPOLARW[0], dPOLARV[1],
                  dPOLARW[1], dPOLARV[2], dPOLARW[2], GlidePolar::WingArea);
              #endif

		if (ww[0]<=0 || dPOLARV[0]==0 || dPOLARW[0]==0 || dPOLARV[1]==0 || dPOLARW[1]==0 || dPOLARV[2]==0 || dPOLARW[2]==0) {
			// StartupStore(_T("... WARNING found invalid Polar line, skipping%s"),NEWLINE);
			continue; // read another line searching for polar
		} else {
			if (GlidePolar::WingArea == 0) {
				StartupStore(_T("... WARNING Polar file has NO wing area%s"),NEWLINE);
			}
			foundline = PolarWinPilot2XCSoar(dPOLARV, dPOLARW, ww);
		}
            }
        }

	int i;

	// Reset flaps values after loading a new polar, and init FlapsPos for the first time
	for (i=0; i<MAX_FLAPS; i++) {
		GlidePolar::FlapsPos[i]=0.0;
		_tcscpy(GlidePolar::FlapsName[i],_T("???"));
	}
	GlidePolar::FlapsPosCount=0;
	GlidePolar::FlapsMass=0.0;

	// Unless we check valid string, even with empty string currentFlapsPos will be positive,
	// and thus force Flaps calculations even with no extended polar.
	// Let's allow empty lines and comments in the polar file, before the flaps line is found.
	//
	do {
	   if (_tcslen(TempString) <10) continue;
	   if(_tcsstr(TempString,TEXT("*")) == TempString) continue;
	   // try to read flaps configuration line
	   PExtractParameter(TempString, ctemp, 0);
	   GlidePolar::FlapsMass = StrToDouble(ctemp,NULL);
	   PExtractParameter(TempString, ctemp, 1);
	   int flapsCount = (int) StrToDouble(ctemp,NULL);

	   // int currentFlapsPos = 0;
	   // GlidePolar::FlapsPos[currentFlapsPos][0] = 0.0;  // no need, already initialised

	   int currentFlapsPos=1;
	   for (i=2; i <= flapsCount*2; i=i+2) {
	        PExtractParameter(TempString, ctemp, i);
	        GlidePolar::FlapsPos[currentFlapsPos] = StrToDouble(ctemp,NULL);
			if (GlidePolar::FlapsPos[currentFlapsPos] > 0) {
			  GlidePolar::FlapsPos[currentFlapsPos] = GlidePolar::FlapsPos[currentFlapsPos]/TOKPH;
			}
	        PExtractParameter(TempString, ctemp, i+1);
		ctemp[MAXFLAPSNAME]='\0';
		if (ctemp[_tcslen(ctemp)-1]=='\r' || ctemp[_tcslen(ctemp)-1]=='\n')
			ctemp[_tcslen(ctemp)-1]='\0'; // remove trailing cr
		_tcscpy(GlidePolar::FlapsName[currentFlapsPos],ctemp);
		if (currentFlapsPos >= (MAX_FLAPS-1)) break; // safe check
	        currentFlapsPos++;
	   }
	   _tcscpy(GlidePolar::FlapsName[0],GlidePolar::FlapsName[1]);
           GlidePolar::FlapsPos[currentFlapsPos] = MAXSPEED;
           _tcscpy(GlidePolar::FlapsName[currentFlapsPos],ctemp);
           currentFlapsPos++;
           GlidePolar::FlapsPosCount = currentFlapsPos;
	   break;
	} while(stream.read_line(TempString));

	stream.close();
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
		GlidePolar::WingArea = 10.04;
		gcc_unused bool bok = PolarWinPilot2XCSoar(dPOLARV, dPOLARW, ww);
		assert(bok);
		_tcscpy(szPolarFile,_T(LKD_DEFAULT_POLAR));

		MessageBoxX(MsgToken(920), // Error loading Polar file!
								MsgToken(791), // Warning
								mbOk);
	} // !foundline

	GlidePolar::SetBallast();

	return(foundline);
}
