#include "map_renderer.h"
#include <unordered_map>

inline constexpr const char *MAP_ROUTE_NAME_FONT_FAMILY = "Verdana";
inline constexpr const char *MAP_ROUTE_NAME_FONT_WEIGHT = "bold";

using namespace std;
using namespace transport;
using namespace transport::renderer;

bool transport::renderer::IsZero(double value) {
  return std::abs(value) < EPSILON;
}

/*--------------------------- SphereProjector --------------------------------*/
svg::Point transport::renderer::SphereProjector::operator()(
    transport::detail::Coordinates coords) const {
  return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
          (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

/* ----------------------- Созидатель карты -------------------------------- */
void transport::MapRenderer_impl::SetSettings(
    const renderer::RenderSettings &settings) {
  if (settings.isValid()) {
    settings_ = settings;
  }
  //  cout << "Установленны на стройки MapRenderer" << endl;
}

bool MapRenderer_impl::hasSettings() { return settings_.isValid(); }

svg::Polyline MapRenderer_impl::createDefaultRoute_() const {
  return svg::Polyline()
      .SetFillColor(svg::NoneColor)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeWidth(settings_.lineWidth());
}

svg::Text MapRenderer_impl::createDefaultRouteName_() const {
  return svg::Text()
      .SetOffset(settings_.busLabelOffset())
      .SetFontSize(settings_.busLabelFontSize())
      .SetFontFamily(MAP_ROUTE_NAME_FONT_FAMILY)
      .SetFontWeight(MAP_ROUTE_NAME_FONT_WEIGHT);
}

svg::Text MapRenderer_impl::createDefaultRouteName_underlayer_() const {
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

svg::Text MapRenderer_impl::createDefaultStopName_() const {
  return svg::Text()
      .SetOffset(settings_.stopLabelOffset())
      .SetFontSize(settings_.stopLabelFontSize())
      .SetFontFamily(MAP_ROUTE_NAME_FONT_FAMILY)
      .SetFillColor("black");
}

svg::Text MapRenderer_impl::createDefaultStopName_underlayer_() const {
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

svg::Document
MapRenderer_impl::renderRoutesMap(const std::vector<BusInfo> &bus_info) const {

  if (bus_info.empty()) {
    return {};
  }
  settings_.resetColorPalette();

  std::vector<BusInfo> bus_routes(bus_info);

  // Построчное создание картинки svg
  stable_sort(bus_routes.begin(), bus_routes.end(),
              [](const BusInfo &lhs, const BusInfo &rhs) {
                return lhs.name < rhs.name;
              });
  // Сливаю координаты в один вектор
  vector<detail::Coordinates> geo_coords;
  for (const auto &route : bus_routes) {
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
  std::unordered_map<std::string_view, svg::Color> bus_colors{};
  // СЛОЙ 1. Ломаные линии маршрутов
  for (BusInfo &route : bus_routes) {
    if (!route.stops.empty()) {
      svg::Color bus_color = settings_.nextColor();
      svg::Polyline route_polyline(
          createDefaultRoute_().SetStrokeColor(bus_color));

      //туда
      for_each(route.stops.begin(), route.stops.end(),
               [&](const auto &stop_info) {
                 svg::Point stop_point = proj(stop_info.coordinates);
                 route_polyline.AddPoint(stop_point);
                 stops.emplace_back(stop_info.name, stop_point);
               });
      if (!route.is_roundtrip) {
        // и обратно
        for_each(next(route.stops.rbegin()), route.stops.rend(),
                 [&](const auto &stop_info) {
                   svg::Point stop_point = proj(stop_info.coordinates);
                   route_polyline.AddPoint(stop_point);
                   stops.emplace_back(stop_info.name, stop_point);
                 });
      }
      doc.Add(route_polyline);
      bus_colors.emplace(route.name, bus_color);
    }
  }
  // СЛОЙ 2. Названия маршрутов
  for (const BusInfo &route : bus_routes) {
    svg::Color bus_color = bus_colors.at(route.name);
    svg::Point first_stop_point = proj(route.stops.front().coordinates);
    doc.Add(svg::Text(createDefaultRouteName_underlayer_()
                          .SetData(string(route.name))
                          .SetPosition(first_stop_point)));

    doc.Add(svg::Text(createDefaultRouteName_()
                          .SetPosition(first_stop_point)
                          .SetFillColor(bus_color)
                          .SetData(string(route.name))
                          .SetFillColor(bus_color)));
    svg::Point second_stop_point = proj(route.stops.back().coordinates);

    if (!route.is_roundtrip &&
        route.stops.front().name != route.stops.back().name) {

      doc.Add(svg::Text(createDefaultRouteName_underlayer_()
                            .SetData(string(route.name))
                            .SetPosition(second_stop_point)));

      doc.Add(svg::Text(createDefaultRouteName_()
                            .SetData(string(route.name))
                            .SetPosition(second_stop_point)
                            .SetFillColor(bus_color)));
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

std::unique_ptr<MapRenderer> MapRenderer::Make() {
  return std::make_unique<MapRenderer_impl>();
}
