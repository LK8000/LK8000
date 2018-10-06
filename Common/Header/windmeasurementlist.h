/***********************************************************************
**
**   windmeasurementlist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id: windmeasurementlist.h,v 1.1 2011/12/21 10:35:29 root Exp root $
**
***********************************************************************/

#ifndef WINDMEASUREMENTLIST_H
#define WINDMEASUREMENTLIST_H

/**The WindMeasurementList is a list that can contain and process windmeasurements.
  *@author André Somers
  */

#include "vector.h"

struct WindMeasurement {
  Vector vector;
  int quality;
  long time;
  double altitude;
};

#define MAX_MEASUREMENTS 200
//maximum number of windmeasurements in the list. No idea what a sensible value would be...

class WindMeasurementList {
public:
  WindMeasurementList();
  virtual ~WindMeasurementList();

  /**
   * Returns the weighted mean windvector over the stored values, or 0
   * if no valid vector could be calculated (for instance: too little or
   * too low quality data).
   */
  Vector getWind(double Time, double alt, bool *found);
  /** Adds the windvector vector with quality quality to the list. */
  void addMeasurement(double Time, Vector vector, double alt, int quality);

protected:
  WindMeasurement *measurementlist[MAX_MEASUREMENTS];
  unsigned int nummeasurementlist;

  /**
   * getLeastImportantItem is called to identify the item that should be
   * removed if the list is too full. Reimplemented from LimitedList.
   */
  virtual unsigned int getLeastImportantItem(double Time);

 private:
};

#endif
