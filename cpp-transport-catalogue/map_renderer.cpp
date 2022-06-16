#include "map_renderer.h"
#include <unordered_map>

inline constexpr const char *MAP_ROUTE_NAME_FONT_FAMILY = "Verdana";
inline constexpr const char *MAP_ROUTE_NAME_FONT_WEIGHT = "bold";

using namespace std;
using namespace transport;
namespace transport::renderer {

bool IsZero(double value) { return std::abs(value) < EPSILON; }

/*--------------------------- SphereProjector --------------------------------*/
svg::Point
SphereProjector::operator()(transport::detail::Coordinates coords) const {
  return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
          (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}
} // namespace transport::renderer
/* ----------------------- Созидатель карты -------------------------------- */
void transport::MapRenderer::SetSettings(
    const renderer::RenderSettings &settings) {
  if (settings.isValid()) {
    settings_ = settings;
  }
}

bool MapRenderer::hasSettings() { return settings_.isValid(); }

svg::Polyline MapRenderer::createDefaultRoute_() const {
  return svg::Polyline()
      .SetFillColor(svg::NoneColor)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeWidth(settings_.lineWidth());
}

svg::Text MapRenderer::createDefaultRouteName_() const {
  return svg::Text()
      .SetOffset(settings_.busLabelOffset())
      .SetFontSize(settings_.busLabelFontSize())
      .SetFontFamily(MAP_ROUTE_NAME_FONT_FAMILY)
      .SetFontWeight(MAP_ROUTE_NAME_FONT_WEIGHT);
}

svg::Text MapRenderer::createDefaultRouteName_underlayer_() const {
  return svg::Text()
      .SetOffset(settings_.busLabelOffset())
      .SetFontSize(settings_.busLabelFontSize())
      .SetFontFamily(MAP_ROUTE_NAME_FONT_FAMILY)
      .SetFontWeight(MAP_ROUTE_NAME_FONT_WEIGHT)
      .SetFillColor(settings_.underlayerColor())
      .SetStrokeColor(settings_.underlayerColor())
      .SetStrokeWidth(settings_.underlayerWidth())
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

svg::Text MapRenderer::createDefaultStopName_() const {
  return svg::Text()
      .SetOffset(settings_.stopLabelOffset())
      .SetFontSize(settings_.stopLabelFontSize())
      .SetFontFamily(MAP_ROUTE_NAME_FONT_FAMILY)
      .SetFillColor("black");
}

svg::Text MapRenderer::createDefaultStopName_underlayer_() const {
  return svg::Text()
      .SetOffset(settings_.stopLabelOffset())
      .SetFontSize(settings_.stopLabelFontSize())
      .SetFontFamily(MAP_ROUTE_NAME_FONT_FAMILY)
      .SetFillColor(settings_.underlayerColor())
      .SetStrokeColor(settings_.underlayerColor())
      .SetStrokeWidth(settings_.underlayerWidth())
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

svg::Document MapRenderer::renderRoutesMap(std::vector<BusInfo> routes) const {

  if (routes.empty()) {
    return {};
  }
  settings_.resetColorPalette();
  // Построчное создание картинки svg
  stable_sort(routes.begin(), routes.end(),
              [](const BusInfo &lhv, const BusInfo &rhv) {
                return lhv.name < rhv.name;
              });
  // Сливаю координаты в один вектор
  vector<detail::Coordinates> geo_coords;
  for (const auto &route : routes) {
    transform(route.stops.begin(), route.stops.end(), back_inserter(geo_coords),
              [](const StopInfo &stop_info) { return stop_info.coordinates; });
  }
  // Создаём проектор сферических координат на карту
  const renderer::SphereProjector proj{geo_coords.begin(), geo_coords.end(),
                                       settings_.width(), settings_.height(),
                                       settings_.padding()};

  // Вывожу заготовку для svg|xml
  svg::Document doc;
  std::vector<std::pair<std::string_view, svg::Point>> stops;
  // СЛОЙ 1. Ломаные линии маршрутов
  for (BusInfo &route : routes) {
    if (0U != route.stops.size()) {
      route.color = settings_.nextColor();
      svg::Polyline route_polyline(
          createDefaultRoute_().SetStrokeColor(route.color));
      for (const auto &stop_info : route.stops) {
        svg::Point stop_point = proj(stop_info.coordinates);
        route_polyline.AddPoint(stop_point);
        stops.emplace_back(stop_info.name, stop_point);
      }
      doc.Add(route_polyline);
    }
  }
  // СЛОЙ 2. Названия маршрутов
  for (const BusInfo &route : routes) {
    svg::Point first_stop_point = proj(route.stops.front().coordinates);
    doc.Add(svg::Text(createDefaultRouteName_underlayer_()
                          .SetData(route.name)
                          .SetPosition(first_stop_point)));

    doc.Add(svg::Text(createDefaultRouteName_()
                          .SetPosition(first_stop_point)
                          .SetFillColor(route.color)
                          .SetData(route.name)
                          .SetFillColor(route.color)));
    svg::Point second_stop_point =
        proj(route.stops.at(route.stops.size() / 2).coordinates);

    if (!route.is_roundtrip &&
        route.stops.front().name !=
            route.stops.at(route.stops.size() / 2).name) {

      doc.Add(svg::Text(createDefaultRouteName_underlayer_()
                            .SetData(route.name)
                            .SetPosition(second_stop_point)));

      doc.Add(svg::Text(createDefaultRouteName_()
                            .SetData(route.name)
                            .SetPosition(second_stop_point)
                            .SetFillColor(route.color)));
    }
  }

  stable_sort(stops.begin(), stops.end(), [](const auto &lhv, const auto &rhv) {
    return lhv.first < rhv.first;
  });
  auto last =
      unique(stops.begin(), stops.end(), [](const auto &lhv, const auto &rhv) {
        return lhv.first == rhv.first;
      });
  stops.erase(last, stops.end());

  // СЛОЙ 3. Круги, обозначающие остановки
  for (const auto &[name, stop_point] : stops) {

    doc.Add(svg::Circle()
                .SetCenter(stop_point)
                .SetRadius(settings_.stopRadius())
                .SetFillColor("white"));
  }

  // СЛОЙ 4. Названия остановок
  for (const auto &[name, stop_point] : stops) {
    doc.Add(svg::Text(createDefaultStopName_underlayer_()
                          .SetPosition(stop_point)
                          .SetData(string(name))));
    doc.Add(svg::Text(createDefaultStopName_()
                          .SetPosition(stop_point)
                          .SetData(string(name))));
  }

  // Завершаю "рисование"
  return doc;
}
