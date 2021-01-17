//
// Created by BLC on 27/10/2020.
//

#ifndef GEOGRAPHIC_TRANSVERSEMERCATOR_H
#define GEOGRAPHIC_TRANSVERSEMERCATOR_H

#include "GeoPoint.h"

class TransverseMercator final {
public:
  TransverseMercator() = delete;
  explicit TransverseMercator(const GeoPoint& center);

  GeoPoint Reverse(double N, double E) const;
  void Forward(const GeoPoint& position, double& N, double& E) const;

protected:
  const double lat0;
  const double lon0;

  const double k0;

  const double N0;
  const double E0;
};

#endif //ANDROID_STUDIO_TRANSVERSEMERCATOR_H
