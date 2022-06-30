/*
 * Здесь можно разместить код транспортного справочника
 */
#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <string>

using namespace std;

namespace transport {
TransportCatalogue::Stop::Stop(const std::string &name) : name(name) {}

TransportCatalogue::Stop::Stop(const std::string &name,
                               const detail::Coordinates coordinates)
    : name(name), coordinates(coordinates) {}

TransportCatalogue::Bus::Bus(const std::string &name,
                             const std::vector<Stop *> &stops)
    : name(name), stops(stops) {}

void TransportCatalogue::addBus(std::string_view busName,
                                const vector<string_view> &stops,
                                bool is_roundtrip) {

  vector<Stop *> bus_stops;
  for_each(stops.begin(), stops.end(), GetKnownStop(bus_stops, this));

  buses_.emplace_back(string(busName), bus_stops);
  Bus &new_bus = buses_.back();
  for_each(new_bus.stops.begin(), new_bus.stops.end(),
           [&new_bus](Stop *stop) { stop->buses.emplace(new_bus.name); });
  new_bus.is_roundtrip = is_roundtrip;
  busesIndex_.emplace(new_bus.name, &new_bus);
}

size_t TransportCatalogue::getBusesCount() const { return buses_.size(); }

TransportCatalogue::Stop *
TransportCatalogue::addStop_(std::string_view stop_name) {
  auto stop = findStop(stop_name);
  if (!stop) {
    stops_.emplace_back(string(stop_name));
    stop = optional<Stop *>{&stops_.back()};
    stopsIndex_.emplace(stop.value()->name, (*stop));
  }
  return *stop;
}

void TransportCatalogue::addStop(
    std::string_view name, detail::Coordinates coordinates,
    std::unordered_map<string_view, int> &nearbyStops) {
  Stop *stop = addStop_(name);
  // остановка обновлена или создана
  (*stop).coordinates.emplace(coordinates);
  // рассчет географических расстояний от новой остановки до остальных и от
  // остальных до новой. Данная вставка позволяет иметь сложность метода
  // getBusInfo О(1) (амортизированную), так как большинство расстояний будут
  // подсчитаны, однако сложность добавления остановки составит тогда O(N), где
  // N - количество имеющихся остановок
  //  for_each(stops_.begin(), stops_.end(), [stop, this](const Stop
  //  &second_stop) {
  //    if (!second_stop.coordinates) {
  //      return;
  //    }
  //    const auto iter = geoDistances_.find({stop, &second_stop});
  //    if (iter == geoDistances_.end()) {
  //      // рассчитать расстояние
  //      double result = ComputeDistance(stop->coordinates.value(),
  //                                      second_stop.coordinates.value());
  //      geoDistances_.emplace(make_pair(stop, &second_stop), result);
  //      geoDistances_.emplace(make_pair(&second_stop, stop), result);
  //    }
  //  });

  // добавляю данные в routeDistances_, попутно создавая новые остановки
  if (!nearbyStops.empty()) {
    for (auto [stop_name, distance] : nearbyStops) {
      Stop *nearbyStop = addStop_(stop_name);
      routeDistances_.emplace(make_pair(stop, nearbyStop), distance);
    }
  }
}

size_t TransportCatalogue::getStopsCount() const { return stops_.size(); }

optional<TransportCatalogue::Bus *>
TransportCatalogue::findBus(std::string_view bus_name) const {
  auto iter = busesIndex_.find(bus_name);
  return (busesIndex_.find(bus_name) == busesIndex_.end())
             ? nullopt
             : std::optional<Bus *>{iter->second};
}

optional<TransportCatalogue::Stop *>
TransportCatalogue::findStop(std::string_view stop_name) const {
  if (auto iter = stopsIndex_.find(stop_name); iter != stopsIndex_.end()) {
    return {iter->second};
  }
  return nullopt;
}

optional<BusStat>
TransportCatalogue::getBusStat(std::string_view bus_name) const {
  if (auto bus = findBus(bus_name); bus) {
    vector<Stop *> stops((*bus)->stops);

    vector<double> geoDistances{};
    /* согласно рекомендации @Савва: "Агрегатная и временная информация
    не должна считаться просто так. Если был запрос на информацию о остановке ли
    маршруте: считаем кол-во остановок, длину маршрута, кривизну итд"
    (https://yandex-students.slack.com/archives/C02T0G67PC4/p1654018639507289?thread_ts=1652686559.771829&cid=C02T0G67PC4)
    и продолжение: "Да, получается, что если мы запросили информацию только об
    одном маршруте, а их 10000, то ты зря потратил время для информации для 9999
    марщрутов"
    (https://yandex-students.slack.com/archives/C02T0G67PC4/p1654019099579329?thread_ts=1652686559.771829&cid=C02T0G67PC4)
    */
    // При большом количестве запросов расстояния между остановками уже будут
    // посчитаны и сохранены в geoDistances_
    transform(stops.begin(), prev(stops.end()), next(stops.begin()),
              back_inserter(geoDistances), GetGeoDistance(geoDistances_));

    vector<double> route_distances{};
    transform(stops.begin(), prev(stops.end()), next(stops.begin()),
              back_inserter(route_distances),
              GetRouteDistance(routeDistances_));

    BusStat bus_info(bus_name);
    bus_info.stops = stops.size();
    sort(stops.begin(), stops.end());

    auto new_end = unique(stops.begin(), stops.end());
    stops.erase(new_end, stops.end());
    bus_info.unique_stops = stops.size();
    bus_info.geolength =
        accumulate(geoDistances.begin(), geoDistances.end(), .0);
    bus_info.routelength =
        accumulate(route_distances.begin(), route_distances.end(), .0);
    return {bus_info};
  }
  return nullopt;
}

std::optional<StopStat>
TransportCatalogue::getStopStat(std::string_view stop_name) const {
  if (auto stop = findStop(stop_name); stop) {
    vector<string_view> buses(stop.value()->buses.begin(),
                              stop.value()->buses.end());
    StopStat stop_info(string(stop_name), buses);
    return stop_info;
  };
  return {};
}

std::optional<std::vector<BusInfo>> TransportCatalogue::getRoutesInfo() const {
  // нужны названия маршрутов (автобусов) с перечнем их остановок (по порядку) с
  // координатами
  if (buses_.empty()) {
    return nullopt;
  }
  std::vector<BusInfo> result;
  result.reserve(buses_.size());
  for (const auto &bus : buses_) {
    if (!bus.stops.empty()) {
      BusInfo element(bus.name);
      element.stops.reserve(bus.stops.size());
      transform(bus.stops.begin(), bus.stops.end(),
                back_inserter(element.stops), [](const Stop *stop) {
                  return StopInfo(stop->name, stop->coordinates.value());
                });
      element.is_roundtrip = bus.is_roundtrip;
      result.push_back(element);
    }
  }
  return result;
}

// функтор находит остановки по их названию, при необходимости добавляет
// остановки
TransportCatalogue::GetKnownStop::GetKnownStop(
    std::vector<Stop *> &busStops, TransportCatalogue *const catalog)
    : busStops_(busStops), catalog_(catalog) {}

void TransportCatalogue::GetKnownStop::operator()(
    std::string_view stop_name) const {
  auto stop = catalog_->findStop(std::string(stop_name));
  if (!stop) { // если не нашлась - создает новую
    stop = std::optional<Stop *>{catalog_->addStop_(stop_name)};
  }
  busStops_.push_back(*stop);
}

TransportCatalogue::GetGeoDistance::GetGeoDistance(
    TransportCatalogue::DistanceMap &distances)
    : distances_(distances) {}

double
TransportCatalogue::GetGeoDistance::operator()(const Stop *firstStop,
                                               const Stop *secondStop) const {
  double result = .0;
  //  if (!(firstStop->coordinates && secondStop->coordinates)) {
  //    return result;
  //  }
  const auto iter = distances_.find({firstStop, secondStop});
  if (iter == distances_.end() && firstStop->coordinates &&
      secondStop->coordinates) {
    // рассчитать расстояние
    result = detail::ComputeDistance(firstStop->coordinates.value(),
                                     secondStop->coordinates.value());
    distances_.emplace(std::make_pair(firstStop, secondStop), result);

  } else {
    result = iter->second;
  }
  return result;
}
TransportCatalogue::GetRouteDistance::GetRouteDistance(
    const TransportCatalogue::DistanceMap &routeDistances)
    : routeDistances_(routeDistances) {}
double
TransportCatalogue::GetRouteDistance::operator()(const Stop *firstStop,
                                                 const Stop *secondStop) const {
  if (routeDistances_.empty()) {
    return 0.;
  }
  auto iter = routeDistances_.find({firstStop, secondStop});
  if (iter == routeDistances_.end()) {
    iter = routeDistances_.find({secondStop, firstStop});
  }
  return iter->second;
}
} // namespace transport
