#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <string>

using namespace std;

// функтор находит остановки по их названию, при необходимости добавляет
// остановки

namespace transport {
TransportCatalogue::Stop::Stop(const std::string &name) : name(name){};

TransportCatalogue::Stop::Stop(const std::string &name,
                               const detail::Coordinates coordinates)
    : name(name), coordinates(coordinates){};

TransportCatalogue::Bus::Bus(const std::string &name,
                             const std::vector<Stop *> &stops)
    : name(name), stops(stops){};

TransportCatalogue::StopInfo::StopInfo(const std::string &name) : name(name){};
TransportCatalogue::StopInfo::StopInfo(
    const std::string &name, const std::vector<std::string_view> &buses)
    : name(name), buses(buses){};

TransportCatalogue::BusInfo::BusInfo(std::string_view name)
    : name(std::string(name)){};

void TransportCatalogue::addBus(std::string_view busName,
                                const vector<string_view> &stops) {

  vector<Stop *> bus_stops;
  GetKnownStop getKnownStop(bus_stops, this);
  for_each(stops.begin(), stops.end(), getKnownStop);

  buses_.emplace_back(string(busName), bus_stops);
  Bus &new_bus = buses_.back();
  for_each(new_bus.stops.begin(), new_bus.stops.end(),
           [&new_bus](Stop *stop) { stop->buses.emplace(new_bus.name); });
  busesIndex_.emplace(new_bus.name, &new_bus);
}

size_t TransportCatalogue::getBusesCount() const { return buses_.size(); }

TransportCatalogue::Stop *
TransportCatalogue::addStop_(std::string_view stop_name) {
  auto stop = findStop(stop_name);
  if (!stop) {
    stops_.emplace_back(string(stop_name));
    stop = optional<TransportCatalogue::Stop *>{&stops_.back()};
    stopsIndex_.emplace(stop_name, (*stop));
  }
  return *stop;
}

void TransportCatalogue::addStop(
    std::string_view name, detail::Coordinates coordinates,
    std::unordered_map<string_view, unsigned long> &nearbyStops) {
  Stop *stop = addStop_(name);
  // остановка обновлена или создана
  (*stop).coordinates.emplace(coordinates);
  // рассчет географических расстояний от новой остановки до остальных и от
  // остальных до новой. Данная вставка позволяет иметь сложность метода
  // getBusInfo О(1) (амортизированную), так как большинство расстояний будут
  // подсчитаны, однако сложность добавления остановки составит тогда O(N), где
  // N - количество имеющихся остановок
  for_each(stops_.begin(), stops_.end(), [stop, this](const Stop &second_stop) {
    if (!second_stop.coordinates) {
      return;
    }
    const auto iter = geoDistances_.find({stop, &second_stop});
    if (iter == geoDistances_.end()) {
      // рассчитать расстояние
      double result = ComputeDistance(stop->coordinates.value(),
                                      second_stop.coordinates.value());
      geoDistances_.emplace(make_pair(stop, &second_stop), result);
      geoDistances_.emplace(make_pair(&second_stop, stop), result);
    }
  });

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
             : std::optional<TransportCatalogue::Bus *>{iter->second};
}

optional<TransportCatalogue::Stop *>
TransportCatalogue::findStop(std::string_view stop_name) const {
  auto iter = stopsIndex_.find(stop_name);
  if (iter == stopsIndex_.end()) {
    return nullopt;
  } else {
    //    return optional<TransportCatalogue::Stop *>{iter->second};
    return {iter->second};
  }
}

optional<TransportCatalogue::BusInfo>
TransportCatalogue::getBusInfo(std::string_view bus_name) const {
  if (auto bus = findBus(bus_name); bus) {
    vector<Stop *> stops((*bus)->stops);
    vector<double> geoDistances{};
    GetGeoDistance getGeoDistance(geoDistances_);
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
              back_inserter(geoDistances), getGeoDistance);
    vector<unsigned long> distances{};
    transform(stops.begin(), prev(stops.end()), next(stops.begin()),
              back_inserter(distances), GetDistance(routeDistances_));
    BusInfo bus_info(bus_name);
    bus_info.stops = stops.size();
    sort(stops.begin(), stops.end());
    auto new_end = unique(stops.begin(), stops.end());
    stops.erase(new_end, stops.end());
    bus_info.unique_stops = stops.size();
    bus_info.geolength =
        accumulate(geoDistances.begin(), geoDistances.end(), .0);
    bus_info.routelength = accumulate(distances.begin(), distances.end(), 0);
    return {bus_info};
  } else {
    return nullopt;
  }
}

std::optional<TransportCatalogue::StopInfo>
TransportCatalogue::getStopInfo(std::string_view stop_name) const {
  if (auto stop = findStop(stop_name); stop) {
    vector<string_view> buses(stop.value()->buses.begin(),
                              stop.value()->buses.end());
    StopInfo stop_info(string(stop_name), buses);
    return stop_info;
  };
  return {};
}

TransportCatalogue::GetKnownStop::GetKnownStop(
    std::vector<TransportCatalogue::Stop *> &busStops,
    TransportCatalogue *const catalog)
    : busStops_(busStops), catalog_(catalog){};
void TransportCatalogue::GetKnownStop::operator()(
    std::string_view stop_name) const {
  auto stop = catalog_->findStop(std::string(stop_name));
  if (!stop) { // если не нашлась - создает новую
    stop = std::optional<TransportCatalogue::Stop *>{
        catalog_->addStop_(stop_name)};
  }
  busStops_.push_back(*stop);
}
TransportCatalogue::GetGeoDistance::GetGeoDistance(
    TransportCatalogue::DistanceMap &distances)
    : distances_(distances) {}
double TransportCatalogue::GetGeoDistance::operator()(
    const TransportCatalogue::Stop *firstStop,
    const TransportCatalogue::Stop *secondStop) const {
  double result = .0;
  const auto iter = distances_.find({firstStop, secondStop});
  if (iter == distances_.end() && firstStop->coordinates &&
      secondStop->coordinates) {
    // рассчитать расстояние
    result = ComputeDistance(firstStop->coordinates.value(),
                             secondStop->coordinates.value());
    distances_.emplace(std::make_pair(firstStop, secondStop), result);

  } else {
    result = iter->second;
  }
  return result;
}
TransportCatalogue::GetDistance::GetDistance(
    const TransportCatalogue::DistanceMap &routeDistances)
    : routeDistances_(routeDistances) {}
unsigned long TransportCatalogue::GetDistance::operator()(
    const TransportCatalogue::Stop *firstStop,
    const TransportCatalogue::Stop *secondStop) const {
  if (routeDistances_.empty()) {
    return 0;
  }
  auto iter = routeDistances_.find({firstStop, secondStop});
  if (iter == routeDistances_.end()) {
    iter = routeDistances_.find({secondStop, firstStop});
  }
  return iter->second;
}
} // namespace transport
