#pragma once

#include "svg.h"
#include "domain.h"
#include "geo.h"
#include <algorithm>
#include <array>
#include <optional>
#include <vector>

inline constexpr double EPSILON = 1e-6;

namespace transport::renderer {
bool IsZero(double value);

/* --------------- Проектировщик со сферы на плоскость ---------------------- */
class SphereProjector {
public:
  // points_begin и points_end задают начало и конец интервала элементов
  // geo::Coordinates
  template <typename PointInputIt>
  SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                  double max_width, double max_height, double padding)
      : padding_(padding) //
  {
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
      return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] =
        std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
          return lhs.lng < rhs.lng;
        });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] =
        std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
          return lhs.lat < rhs.lat;
        });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
      width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
      height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
      // Коэффициенты масштабирования по ширине и высоте ненулевые,
      // берём минимальный из них
      zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
      // Коэффициент масштабирования по ширине ненулевой, используем его
      zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
      // Коэффициент масштабирования по высоте ненулевой, используем его
      zoom_coeff_ = *height_zoom;
    }
  }

  // Проецирует широту и долготу в координаты внутри SVG-изображения
  svg::Point operator()(transport::detail::Coordinates coords) const;

private:
  double padding_;
  double min_lon_ = 0;
  double max_lat_ = 0;
  double zoom_coeff_ = 0;
};

} // namespace transport::renderer
namespace transport {
/* ----------------------- Созидатель карты -------------------------------- */
class MapRenderer {
public:
  virtual ~MapRenderer() = default;
  virtual void SetSettings(const renderer::RenderSettings &) = 0;
  [[nodiscard]] virtual bool hasSettings() = 0;
  [[nodiscard]] virtual svg::Document
  renderRoutesMap(const std::vector<BusInfo> &bus_info) const = 0;

  static std::unique_ptr<MapRenderer> Make();
};

class MapRenderer_impl : public MapRenderer {
public:
  void SetSettings(const renderer::RenderSettings &) override;
  [[nodiscard]] bool hasSettings() override;
  [[nodiscard]] svg::Document
  renderRoutesMap(const std::vector<BusInfo> &bus_info) const override;

private:
  svg::Polyline createDefaultRoute_() const;
  svg::Text createDefaultRouteName_() const;
  svg::Text createDefaultRouteName_underlayer_() const;
  svg::Text createDefaultStopName_() const;
  svg::Text createDefaultStopName_underlayer_() const;

  renderer::RenderSettings settings_{};
  std::vector<BusInfo> bus_routes_{};
};
} // namespace transport
