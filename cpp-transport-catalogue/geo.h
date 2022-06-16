#pragma once

namespace transport::detail {

struct Coordinates {
  double lat; // Широта
  double lng; // Долгота
  bool operator==(Coordinates other) const;
  bool operator!=(Coordinates other) const;
};

double ComputeDistance(Coordinates, Coordinates);

} // namespace transport::detail
