#include "geo.h"

#define USE_MATH_DEFINES
#include <cmath>

using namespace std;

namespace transport {
namespace detail {
constexpr double EARTH_RADIUS = 6371000.;
constexpr double EPSILON = 1.e-6;

double ComputeDistance(Coordinates distance_from, Coordinates distance_to) {
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
  return std::abs(lat - other.lat) < EPSILON &&
         std::abs(lng - other.lng) < EPSILON;
}

bool Coordinates::operator!=(Coordinates other) const {
  return !(*this == other);
}
} // namespace detail
} // namespace transport
