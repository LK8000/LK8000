/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <functional>
using std::placeholders::_1;

double MapWindow::LimitMapScale(double value) {

  double minreasonable= MapWindow::zoom.GetZoomInitValue(0)*1000;

  if ( !(MapWindow::zoom.CircleZoom() &&
      mode.Is(Mode::MODE_CIRCLING))
      && zoom.AutoZoom() &&
      mode.Special() == Mode::MODE_SPECIAL_NONE ) {
      minreasonable = MapWindow::zoom.GetZoomInitValue(MaxAutoZoom) * 1000;
  }

  minreasonable = Units::ToDistance(minreasonable);

  // return value in user distance units!!!
  if (ScaleListCount>0) {
    return FindMapScale(max(minreasonable,min(160.0,value)));		//maximum limit in user distance units!
  } else {
    return max(minreasonable,min(160.0,value));				//maximum limit in user distance units!
  }
}


//
// DO NOT CHANGE. This is the unity upon which rescaling relies on.
// It is like 0 degrees Celsius.
//
int MapWindow::GetMapResolutionFactor(void) {
  return NIBLSCALE(30);
}


double MapWindow::StepMapScale(int Step){
  if (abs(Step)>=4) {
    ScaleCurrent += Step/4;
  } else {
    ScaleCurrent += Step;
  }
  LKASSERT(DrawRect.right!=0);
  ScaleCurrent = max(0,min(ScaleListCount-1, ScaleCurrent));
  return((ScaleList[ScaleCurrent]*GetMapResolutionFactor())
         /(IBLSCALE(DrawRect.right)));
}




double MapWindow::FindMapScale(double Value){

  int    i;
  double BestFit = 99999;
  int    BestFitIdx=-1;
  LKASSERT(DrawRect.right!=0);
  LKASSERT(GetMapResolutionFactor()!=0);
  double DesiredScale =
    (Value*IBLSCALE(DrawRect.right))/GetMapResolutionFactor();

  LKASSERT(DesiredScale!=0);
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
           /IBLSCALE(DrawRect.right));
  }
  return(Value);
}

int MapWindow::GetScaleListCount()
{
  return ScaleListCount;
}

// Fill up discrete map scales
void MapWindow::FillScaleListForEngineeringUnits() {

  const double scalefactor = (double) GetMapResolutionFactor() / (double) IBLSCALE(DrawRect.right);

  auto apply_scale = std::bind(std::divides<double>(), _1, scalefactor);

  switch (Units::GetDistanceUnit()) {
    default:
      std::transform(std::begin(ScaleListArrayMeters), std::end(ScaleListArrayMeters),
                     std::begin(ScaleList), apply_scale);
          break;
    case unStatuteMiles:
      std::transform(std::begin(ScaleListArrayStatuteMiles), std::end(ScaleListArrayStatuteMiles),
                     std::begin(ScaleList), apply_scale);
          break;

    case unNauticalMiles:
      std::transform(std::begin(ScaleListNauticalMiles), std::end(ScaleListNauticalMiles),
                     std::begin(ScaleList), apply_scale);
          break;
  } //sw units

  ScaleListCount = SCALELISTSIZE;  // ToDo Remove ScaleListCount global variable

}
