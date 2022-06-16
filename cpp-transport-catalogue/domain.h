#pragma once

#include "geo.h"
#include "svg.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace std::string_view_literals;
namespace transport::detail {
template <typename T> void hash_combine(std::size_t &seed, T const &key) {
  std::hash<T> hasher;
  seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct pair_hash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2> &pair) const {
    std::size_t seed1(0);
    hash_combine(seed1, pair.first);
    hash_combine(seed1, pair.second);

    std::size_t seed2(0);
    hash_combine(seed2, pair.second);
    hash_combine(seed2, pair.first);

    return std::min(seed1, seed2);
  }
};
} // namespace transport::detail

namespace transport::renderer {
/* --------------- Структура представляющие параметры рендера --------------- */
class RenderSettings {
public:
  [[nodiscard]] double width() const;
  [[nodiscard]] double height() const;
  [[nodiscard]] double padding() const;
  [[nodiscard]] double lineWidth() const;
  [[nodiscard]] double stopRadius() const;
  [[nodiscard]] int busLabelFontSize() const;
  [[nodiscard]] svg::Point busLabelOffset() const;
  [[nodiscard]] int stopLabelFontSize() const;
  [[nodiscard]] svg::Point stopLabelOffset() const;
  [[nodiscard]] const svg::Color &underlayerColor() const;
  [[nodiscard]] double underlayerWidth() const;
  [[nodiscard]] const svg::Color &nextColor() const;
  void resetColorPalette() const;
  [[nodiscard]] bool isValid() const;

  void setSize(double newWidth, double newHeight, double newPadding);
  void setStopExterior(double newStop_radius, int newStop_label_font_size,
                       std::array<double, 2> newStop_label_offset);

  void setBusExterior(int newBus_label_font_size,
                      std::array<double, 2> newBus_label_offset);

  void setUnderlayerExterior(double newUnderlayer_width,
                             const svg::Color &newUnderlayer_color);

  void setLineWidth(double newLine_width);
  void setColorPalette(const std::vector<svg::Color> &newColor_palette);

private:
  static const int MaxRenderSize = 100'000;
  mutable size_t paletteIndex_{};
  double width_{}; // ширина изображения в пикселях. Вещественное число в
                   // диапазоне от 0 до 100000.
  double height_{}; // высота изображения в пикселях. Вещественное число в
                    // диапазоне от 0 до 100000.
  double padding_{}; // отступ краёв карты от границ SVG-документа. Вещественное
                     // число не меньше 0 и меньше min(width, height)/2
  double line_width_{}; // толщина линий, которыми рисуются автобусные маршруты.
                        // Вещественное число в диапазоне от 0 до 100000.
  double stop_radius_{}; // радиус окружностей, которыми обозначаются остановки.
                         // Вещественное число в диапазоне от 0 до 100000.
  int bus_label_font_size_{}; // размер текста, которым написаны названия
                              // автобусных маршрутов. Целое число в диапазоне
                              // от 0 до 100000.
  std::array<double, 2>
      bus_label_offset_{}; // смещение надписи с названием маршрута относительно
                           // координат конечной остановки на карте. Массив из
                           // двух элементов типа double. Задаёт значения
                           // свойств dx и dy SVG-элемента <text>. Элементы
                           // массива — числа в диапазоне от –100000 до 100000.
  int stop_label_font_size_{}; // размер текста, которым отображаются названия
                               // остановок. Целое число в диапазоне от 0 до
                               // 100000.
  std::array<double, 2>
      stop_label_offset_{}; // смещение названия остановки относительно её
                            // координат на карте. Массив из двух элементов типа
                            // double. Задаёт значения свойств dx и dy
                            // SVG-элемента <text>. Числа в диапазоне от –100000
                            // до 100000.
  svg::Color
      underlayer_color_{}; //цвет подложки под названиями остановок и маршрутов.
  double underlayer_width_{}; // толщина подложки под названиями остановок и
                              // маршрутов. Задаёт значение атрибута
                              // stroke-width элемента <text>. Вещественное
                              // число в диапазоне от 0 до 100000.
  std::vector<svg::Color>
      color_palette_{}; //  цветовая палитра. Непустой массив.
};
} // namespace transport::renderer

namespace transport {

// Запросы на добавление и на извлечение информации используют одну структуру
/* --------------- Структуры представляющие Остановку ----------------------- */
struct StopQuery { // Входной запрос
  explicit StopQuery(std::string_view name);
  const std::string name{};
  double latitude{};
  double longitude{};
  std::map<std::string, int> road_distances{};
};

struct StopStat { // Результат со статистикой
  StopStat() = default;
  explicit StopStat(const std::string &);
  explicit StopStat(const std::string &, const std::vector<std::string_view> &);
  const std::string name{};
  std::vector<std::string> buses{};
};

struct StopInfo {
  StopInfo(const std::string &name, detail::Coordinates coordinates);
  const std::string name{};
  detail::Coordinates coordinates{};
};

/* --------------- Структуры представляющие Автобус ------------------------- */
struct BusQuery { // Входной запрос
  explicit BusQuery(std::string_view name);
  const std::string name{};
  std::vector<std::string> stops{};
  bool is_roundtrip{};
};

struct BusStat { // Результат со статистикой
  BusStat() = default;
  explicit BusStat(std::string_view);
  const std::string name{};
  size_t stops = 0;
  size_t unique_stops = 0;
  double geolength = .0;
  size_t routelength = 0;
};

struct BusInfo {
  explicit BusInfo(const std::string &name);
  std::string name{};
  std::vector<StopInfo> stops{};
  bool is_roundtrip{};
  svg::Color color{};
};
/* --------------- Структуры представляющие карту ----------------------- */
struct MapQuery { // Входной запрос
  explicit MapQuery(std::string_view name);
  std::string name{};
};
struct MapStat { // Результат со статистикой
  MapStat() = default;
  explicit MapStat(std::string_view name);
  const std::string name{};
  std::string map_string{};
};
/* --------------- Запрос ------------------------------------------------ */
//  варианты содержимого в запросах
enum class RequestType { Base, Stat, Render };

using RequestValue = std::variant<std::monostate, StopQuery, BusQuery, MapQuery,
                                  renderer::RenderSettings>;

struct Request { // обёртка запроса для передачи в Транспортный каталог
  Request(const RequestValue &value, int request_id);
  RequestValue value{};
  int requestId{};
};

using Requests = std::vector<Request>;

/* --------------- Ответ ------------------------------------------------- */
// варианты содержимого в ответах
using ResponseValue = std::variant<std::monostate, StopStat, BusStat, MapStat>;

struct Response { // обёртка ответа для получения из Транспортного каталога
  Response(const ResponseValue &value, int request_id);
  ResponseValue value{};
  int requestId{};
};

using Responses = std::vector<Response>;

/* --------------- Вывод в поток ------------------------------------------ */
template <class T>
std::ostream &operator<<(std::ostream &stream, const std::vector<T> &vector_) {
  stream << "{"sv;
  bool first = true;
  for (const auto &item : vector_) {
    if (!first) {
      stream << ", "sv;
    }
    first = false;
    stream << item;
  }
  return stream << "}"sv;
}

// Request to Stream
std::ostream &operator<<(std::ostream &, const BusQuery &);
std::ostream &operator<<(std::ostream &, const StopQuery &);
std::ostream &operator<<(std::ostream &, const RequestValue &);

struct RequestToStream {
  // Построить ноду для StopInfo
  void operator()(const StopQuery &info) const;
  // Построить ноду для StopInfo
  void operator()(const BusQuery &info) const;
  void operator()(const MapQuery &info) const;
  void operator()(const renderer::RenderSettings &info) const;
  void operator()(std::monostate /*unused*/) const;
  std::ostream &ostream;
};

// Response to Stream
std::ostream &operator<<(std::ostream &, const BusStat &);
std::ostream &operator<<(std::ostream &, const StopStat &);
std::ostream &operator<<(std::ostream &, const MapStat &);
std::ostream &operator<<(std::ostream &, const ResponseValue &);
std::ostream &operator<<(std::ostream &, const Response &);

struct ResponseToStream {
  // Построить ноду для StopInfo
  void operator()(const StopStat &info) const;
  // Построить ноду для StopInfo
  void operator()(const BusStat &info) const;
  void operator()(const MapStat &info) const;
  void operator()(std::monostate /*unused*/) const;
  std::ostream &ostream;
};

} // namespace transport
