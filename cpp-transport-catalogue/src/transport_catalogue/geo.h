#pragma once

namespace transport {
namespace detail {

struct Coordinates {
  double lat; // Широта
  double lng; // Долгота
  bool operator==(Coordinates other) const;
  bool operator!=(Coordinates other) const;
};

double ComputeDistance(Coordinates, Coordinates);

} // namespace detail
} // namespace transport
