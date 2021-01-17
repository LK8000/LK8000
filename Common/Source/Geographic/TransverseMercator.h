//
// Created by BLC on 27/10/2020.
//

#ifndef GEOGRAPHIC_TRANSVERSEMERCATOR_H
#define GEOGRAPHIC_TRANSVERSEMERCATOR_H

#include "GeoPoint.h"
#include "Math/Point2D.hpp"

class TransverseMercator final {
public:
  TransverseMercator() = delete;
  explicit TransverseMercator(const GeoPoint& center);

  /**
   * Point2D is used to store flat coordinate
   *  by convention we 'x' is 'E' and y is 'N'
   */
  GeoPoint Reverse(Point2D<double> position) const;
  Point2D<double> Forward(const GeoPoint& position) const;

protected:
  const double lat0;
  const double lon0;

  const double k0;

  const double N0;
  const double E0;
};

#endif //ANDROID_STUDIO_TRANSVERSEMERCATOR_H
