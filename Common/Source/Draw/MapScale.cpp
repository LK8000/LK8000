/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "MapWindow.h"

using std::min;
using std::max;




double MapWindow::LimitMapScale(double value) {
  
  double minreasonable=5.0;		// give minreasonable values in system units, in meters! (what you would like to see on the mapscale)

  if (mode.Is(Mode::MODE_CIRCLING)) {
      // during circling
      minreasonable = 50.0;
      if ( ISPARAGLIDER ) minreasonable = 10.0;
  } else {
      // if not circling
      minreasonable = 100.0;
      if ( ISPARAGLIDER ) minreasonable = 10.0;
      if (zoom.AutoZoom()) {
	if (AATEnabled && (ActiveWayPoint>0)) {
	  if ( ISPARAGLIDER ) minreasonable = 10.0; else minreasonable = 1200.0;
	} else {
	  if ( ISPARAGLIDER ) minreasonable = 10.0; else minreasonable = 600.0; 
	}
      }
  }

  minreasonable = Units::ToUserDistance(minreasonable / 1.4);		// 1.4 for mapscale symbol
  
  // return value in user distance units!!!
  if (ScaleListCount>0) {
    return FindMapScale(max(minreasonable,min(160.0,value)));		//maximum limit in user distance units!
  } else {
    return max(minreasonable,min(160.0,value));				//maximum limit in user distance units!
  }
}




int MapWindow::GetMapResolutionFactor(void) { // TESTFIX 091017 CHECKFIX
  static int retglider=NIBLSCALE(30);
  //static int retpara=IBLSCALE(30);
  //if (ISPARAGLIDER) return retpara; else return retglider;

  return retglider;
}




double MapWindow::StepMapScale(int Step){
  static int nslow=0;
  if (abs(Step)>=4) {
    nslow++;
    //    if (nslow %2 == 0) {
    ScaleCurrent += Step/4;
    //    }
  } else {
    ScaleCurrent += Step;
  }
  ScaleCurrent = max(0,min(ScaleListCount-1, ScaleCurrent));
  return((ScaleList[ScaleCurrent]*GetMapResolutionFactor())
         /(IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right)));
}




double MapWindow::FindMapScale(double Value){

  int    i;
  double BestFit = 99999;
  int    BestFitIdx=-1;
  double DesiredScale = 
    (Value*IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right))/GetMapResolutionFactor();

  for (i=0; i<ScaleListCount; i++){
    double err = fabs(DesiredScale - ScaleList[i])/DesiredScale;
    if (err < BestFit){
      BestFit = err;
      BestFitIdx = i;
    }
  }

  if (BestFitIdx != -1){
    ScaleCurrent = BestFitIdx;
    return((ScaleList[ScaleCurrent]*GetMapResolutionFactor())
           /IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right));
  }
  return(Value);
}

void MapWindow::FillScaleListForEngineeringUnits(void)
{
  int i;

  // Fill up discrete map scales
  // Consider scalelist size!!!
  switch (Units::GetUserDistanceUnit()) {
    default:
      ScaleListCount = 0;
      ScaleList[ScaleListCount++] = 0.01;		// km
      ScaleList[ScaleListCount++] = 0.025;
      ScaleList[ScaleListCount++] = 0.05;
      ScaleList[ScaleListCount++] = 0.075;
      ScaleList[ScaleListCount++] = 0.1;
      ScaleList[ScaleListCount++] = 0.15;
      ScaleList[ScaleListCount++] = 0.2;
      ScaleList[ScaleListCount++] = 0.35;
      ScaleList[ScaleListCount++] = 0.5;
      ScaleList[ScaleListCount++] = 0.75;
      ScaleList[ScaleListCount++] = 1.0;
      ScaleList[ScaleListCount++] = 1.5;
      ScaleList[ScaleListCount++] = 2.0;
      ScaleList[ScaleListCount++] = 3.5;
      ScaleList[ScaleListCount++] = 5.0;
      ScaleList[ScaleListCount++] = 7.5;
      ScaleList[ScaleListCount++] = 10.0;
      ScaleList[ScaleListCount++] = 15.0;
      ScaleList[ScaleListCount++] = 20.0;
      ScaleList[ScaleListCount++] = 25.0;
      ScaleList[ScaleListCount++] = 40.0;
      ScaleList[ScaleListCount++] = 50.0;
      ScaleList[ScaleListCount++] = 75.0;
      break;
      
    case unStatuteMiles:
      ScaleListCount = 0;
      ScaleList[ScaleListCount++] = 50.0 * (0.0006214 / 3.281);		// to ft;
      ScaleList[ScaleListCount++] = 125.0  * (0.0006214 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 250.0 * (0.0006214 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 400.0 * (0.0006214 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 500.0 * (0.0006214 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 800.0 * (0.0006214 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 0.2;
      ScaleList[ScaleListCount++] = 0.35;
      ScaleList[ScaleListCount++] = 0.5;
      ScaleList[ScaleListCount++] = 0.75;
      ScaleList[ScaleListCount++] = 1.0;
      ScaleList[ScaleListCount++] = 1.5;
      ScaleList[ScaleListCount++] = 2.0;
      ScaleList[ScaleListCount++] = 3.5;
      ScaleList[ScaleListCount++] = 5.0;
      ScaleList[ScaleListCount++] = 7.5;
      ScaleList[ScaleListCount++] = 10.0;
      ScaleList[ScaleListCount++] = 15.0;
      ScaleList[ScaleListCount++] = 20.0;
      ScaleList[ScaleListCount++] = 25.0;
      ScaleList[ScaleListCount++] = 40.0;
      break;

    case unNauticalMiles:
      ScaleListCount = 0;
      ScaleList[ScaleListCount++] = 50.0 * (0.00053996 / 3.281);	// to ft;
      ScaleList[ScaleListCount++] = 150.0  * (0.00053996 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 300.0 * (0.00053996 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 500.0 * (0.00053996 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 600.0 * (0.00053996 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 900.0 * (0.00053996 / 3.281);	// to ft
      ScaleList[ScaleListCount++] = 0.2;
      ScaleList[ScaleListCount++] = 0.35;
      ScaleList[ScaleListCount++] = 0.5;
      ScaleList[ScaleListCount++] = 0.75;
      ScaleList[ScaleListCount++] = 1.0;
      ScaleList[ScaleListCount++] = 1.5;
      ScaleList[ScaleListCount++] = 2.0;
      ScaleList[ScaleListCount++] = 3.5;
      ScaleList[ScaleListCount++] = 5.0;
      ScaleList[ScaleListCount++] = 7.5;
      ScaleList[ScaleListCount++] = 10.0;
      ScaleList[ScaleListCount++] = 15.0;
      ScaleList[ScaleListCount++] = 20.0;
      ScaleList[ScaleListCount++] = 25.0;
      ScaleList[ScaleListCount++] = 40.0;
      break;
  } //sw units
  
  double scalefactor = (double)GetMapResolutionFactor() / (double)IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right) * 1.4;
  for (i=0; i<ScaleListCount; i++) ScaleList[i] /= scalefactor;
}
