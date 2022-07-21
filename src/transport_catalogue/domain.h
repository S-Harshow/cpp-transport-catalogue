#pragma once

#include "geo.h"
#include "../svg/svg.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

using namespace std::string_view_literals;
namespace transport {
namespace detail {
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

namespace renderer {
/* --------------- Структура представляющие параметры рендера --------------- */
class RenderSettings {
public:
  [[nodiscard]] double width() const;
  [[nodiscard]] double height() const;
  [[nodiscard]] double padding() const;
  [[nodiscard]] double lineWidth() const;
  [[nodiscard]] double stopRadius() const;
  [[nodiscard]] uint32_t busLabelFontSize() const;
  [[nodiscard]] svg::Point busLabelOffset() const;
  [[nodiscard]] uint32_t stopLabelFontSize() const;
  [[nodiscard]] svg::Point stopLabelOffset() const;
  [[nodiscard]] const svg::Color &underlayerColor() const;
  [[nodiscard]] double underlayerWidth() const;
  [[nodiscard]] const svg::Color &nextColor() const;
  void resetColorPalette() const;
  [[nodiscard]] bool isValid() const;
  [[nodiscard]] operator bool() const;
  [[nodiscard]] operator std::string() const;
  void setSize(double newWidth, double newHeight, double newPadding);
  void setStopExterior(double newStop_radius, uint32_t newStop_label_font_size,
                       std::pair<double, double> newStop_label_offset);

  void setBusExterior(uint32_t newBus_label_font_size,
                      std::pair<double, double> newBus_label_offset);

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
  uint32_t bus_label_font_size_{}; // размер текста, которым написаны названия
                                   // автобусных маршрутов. Целое число в
                                   // диапазоне от 0 до 100000.
  svg::Point
      bus_label_offset_{}; // смещение надписи с названием маршрута относительно
                           // координат конечной остановки на карте. Массив из
                           // двух элементов типа double. Задаёт значения
                           // свойств dx и dy SVG-элемента <text>. Элементы
                           // массива — числа в диапазоне от –100000 до 100000.
  uint32_t stop_label_font_size_{}; // размер текста, которым отображаются
                                    // названия остановок. Целое число в
                                    // диапазоне от 0 до 100000.
  svg::Point
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
}

// Запросы на добавление и на извлечение информации используют одну структуру
/* --------------- Структуры представляющие Остановку ----------------------- */
struct StopData { // Входной запрос
  StopData() = default;
  explicit StopData(const std::string & name);
  std::string name{};
  detail::Coordinates coordinates{};
  std::map<std::string, double> road_distances{};
};

struct StopStat { // Результат со статистикой
  StopStat() = default;
  explicit StopStat(const std::string & name);
  explicit StopStat(const std::string & name, const std::vector<std::string> &);
  std::string name{};
  std::vector<std::string> buses{};
};

struct StopInfo {
  StopInfo(std::string_view name, detail::Coordinates coordinates);
  std::string_view name{};
  detail::Coordinates coordinates{};
};

/* --------------- Структуры представляющие Автобус ------------------------- */
struct BusData { // Входной запрос
  BusData() = default;
  explicit BusData(const std::string & name);
  std::string name{};
  std::vector<std::string> stops{};
  bool is_roundtrip{};
};

struct BusStat { // Результат со статистикой
  BusStat() = default;
  explicit BusStat(const std::string &);
  std::string name;
  uint stops_count{};
  uint unique_stops_count {};
  double geolength {};
  double routelength {};
};

struct BusInfo {
  explicit BusInfo(std::string_view name);
  std::string_view name;
  std::vector<StopInfo> stops;
  bool is_roundtrip{};
};

struct StopPair_hash {
    template <typename T>
    auto operator()(const std::pair<T, T>& pair) const {
        return std::hash<T>{}(pair.first) ^ std::hash<T>{}(pair.second); // not to be used in production (combining hashes using XOR is bad practice)
    }
    using is_transparent = void; // required to make find() work with different type than key_type
};

struct StopPair_equal {
    template <typename A, typename B>
    auto operator()(const std::pair<A, A>& a,
                    const std::pair<B, B>& b) const {
        return a.first == b.first && a.second == b.second;
    }
    using is_transparent = void; // required to make find() work with different type than key_type
};

using StopPair =
    std::pair<std::string_view , std::string_view >;

using Distances =
    std::unordered_map<StopPair, double, StopPair_hash, StopPair_equal>;

/* --------------- Структуры представляющие карту --------------------------- */

struct MapStat { // Результат со статистикой
  MapStat() = default;
  explicit MapStat(const std::string & name);
  std::string name;
  std::string map_string{};
};

/* --------------- Структуры представляющие маршрутизатор ---------------------*/
struct RouteItemStop{
  RouteItemStop(std::string_view name, double wait_time):name(name), wait_time(wait_time){}
  std::string_view  name;
  double wait_time;
};

struct RouteItemBus{
  RouteItemBus(std::string_view name, uint span_count, double time):bus_name(name), span_count(span_count), time(time){}
  std::string_view  bus_name;
  uint span_count;
  double time;
};

using RouteItem = std::variant<RouteItemBus, RouteItemStop>;

template <typename T>
bool is_type(const RouteItem &item){
  return std::holds_alternative<T>(item);
}

struct RouteStat { // Результат со статистикой
  RouteStat() = default;
  double total_time{};
  std::vector<RouteItem> items;
};


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

} // namespace transport
