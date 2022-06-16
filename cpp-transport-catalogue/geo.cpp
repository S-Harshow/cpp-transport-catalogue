#include "geo.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace transport::detail {
const double EARTH_RADIUS = 6371000.;

double ComputeDistance(Coordinates distance_from, Coordinates distance_to) {
  using namespace std;
  if (distance_from == distance_to) {
    return 0;
  }
  static const double dr_ = M_PI / 180.;
  return acos(sin(distance_from.lat * dr_) * sin(distance_to.lat * dr_) +
              cos(distance_from.lat * dr_) * cos(distance_to.lat * dr_) *
                  cos(abs(distance_from.lng - distance_to.lng) * dr_)) *
         EARTH_RADIUS;
}

bool Coordinates::operator==(Coordinates other) const {
  return lat == other.lat && lng == other.lng;
}

bool Coordinates::operator!=(Coordinates other) const {
  return !(*this == other);
}

} // namespace transport::detail
