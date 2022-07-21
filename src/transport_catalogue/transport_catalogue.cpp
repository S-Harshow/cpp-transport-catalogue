#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <execution>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <string>

using namespace std;

using namespace transport;

TransportCatalogue::StopElement::StopElement(const string &name) : name(name) {}

TransportCatalogue::StopElement::StopElement(
    const std::string &name, const detail::Coordinates coordinates)
    : name(name), coordinates(coordinates) {}

TransportCatalogue::BusElement::BusElement(
    const string &name, const std::vector<StopElement *> &stops)
    : name(name), stops(stops) {}

std::unique_ptr<TransportCatalogue> TransportCatalogue::Make() {
  return std::make_unique<TransportCatalogueImpl>();
}

void TransportCatalogueImpl::addBus(const BusData &data) {
  vector<StopElement *> bus_stops;
  bus_stops.reserve(data.stops.size());

  for_each(
      data.stops.begin(), data.stops.end(),
      [this, &data, &bus_stops](const std::string &stop_name) {
        bus_stops.emplace_back(getStop_(stop_name))->buses.emplace(data.name);
      });

  BusElement &new_bus = buses_.emplace_back(data.name, bus_stops);

  new_bus.is_roundtrip = data.is_roundtrip;
  busesIndex_.emplace(new_bus.name, &new_bus);
}

TransportCatalogue::StopElement *
TransportCatalogueImpl::getStop_(std::string_view stop_name) {
  if (auto iter = stops_index_.find(stop_name); iter != stops_index_.end()) {
    return iter->second;
  }
  StopElement *new_stop = &stops_.emplace_back(string(stop_name));
  stops_index_.emplace(new_stop->name, new_stop);
  return new_stop;
}

void TransportCatalogueImpl::addStop(const StopData &stop_data) {
  StopElement *stop = getStop_(stop_data.name);
  // остановка обновлена или создана
  stop->coordinates.emplace(stop_data.coordinates);
  // добавляю данные в routeDistances_, попутно создавая новые остановки
  if (!stop_data.road_distances.empty()) {
    for (const auto &[stop_name, distance] : stop_data.road_distances) {
      StopElement *nearbyStop = getStop_(stop_name);
      routeDistances_[{stop, nearbyStop}] = distance;
    }
  }
}

optional<TransportCatalogue::BusElement *>
TransportCatalogueImpl::findBus(const std::string &bus_name) const {
  if (auto iter = busesIndex_.find(bus_name); iter != busesIndex_.end()) {
    return {iter->second};
  }
  return nullopt;
}

optional<BusStat>
TransportCatalogueImpl::getBusStat(const std::string &bus_name) const {
  if (auto iter = busesIndex_.find(bus_name); iter != busesIndex_.end()) {
    vector<StopElement *> stops(iter->second->stops);

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
    BusStat bus_info((string(bus_name)));

    if (!iter->second->is_roundtrip) {
      transform(stops.rbegin(), prev(stops.rend()), next(stops.rbegin()),
                back_inserter(geoDistances), GetGeoDistance(geoDistances_));
      transform(stops.rbegin(), prev(stops.rend()), next(stops.rbegin()),
                back_inserter(route_distances),
                GetRouteDistance(routeDistances_));
      bus_info.stops_count = static_cast<uint>(stops.size() * 2 - 1);
    } else {
      bus_info.stops_count = static_cast<uint>(stops.size());
    }
    sort(execution::par_unseq, stops.begin(), stops.end());

    auto new_end = unique(stops.begin(), stops.end());
    stops.erase(new_end, stops.end());
    bus_info.unique_stops_count = static_cast<uint>(stops.size());
    bus_info.geolength =
        reduce(execution::par_unseq, geoDistances.begin(), geoDistances.end());
    bus_info.routelength = reduce(execution::par_unseq, route_distances.begin(),
                                  route_distances.end());
    return {bus_info};
  }
  return nullopt;
}

std::optional<StopStat>
TransportCatalogueImpl::getStopStat(const std::string &stop_name) const {
  if (auto iter = stops_index_.find(stop_name); iter != stops_index_.end()) {
    return StopStat(string(stop_name),
                    {iter->second->buses.begin(), iter->second->buses.end()});
  }
  return nullopt;
}

std::optional<std::vector<BusInfo>>
TransportCatalogueImpl::getRoutesInfo() const {
  // нужны названия маршрутов (автобусов) с перечнем их остановок (по порядку)
  // с координатами
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
                back_inserter(element.stops), [](const StopElement *stop) {
                  return StopInfo(stop->name, stop->coordinates.value());
                });
      element.is_roundtrip = bus.is_roundtrip;
      result.emplace_back(element);
    }
  }
  return result;
}

Distances TransportCatalogueImpl::getDistances() const {
  Distances result{};
  result.reserve(routeDistances_.size());
  for (const auto &[stops_pair, distance] : routeDistances_) {
    result[{stops_pair.first->name, stops_pair.second->name}] = distance;
  }
  return result;
}

size_t TransportCatalogueImpl::getStopCount() const { return stops_.size(); }

TransportCatalogueImpl::GetGeoDistance::GetGeoDistance(DistanceMap &distances)
    : distances_(distances) {}

double TransportCatalogueImpl::GetGeoDistance::operator()(
    const StopElement *firstStop, const StopElement *secondStop) const {
  double result = .0;
  if (!firstStop->coordinates || !secondStop->coordinates) {
    return result;
  }

  if (const auto iter = distances_.find({firstStop, secondStop});
      iter != distances_.end()) {
    result = iter->second;
    return result;
  }

  // рассчитать расстояние сначала обтратное расстояние
  result = detail::ComputeDistance(secondStop->coordinates.value(),
                                   firstStop->coordinates.value());
  distances_.emplace(std::make_pair(secondStop, firstStop), result);

  // потом прямое - искомое
  result = detail::ComputeDistance(firstStop->coordinates.value(),
                                   secondStop->coordinates.value());
  distances_.emplace(std::make_pair(firstStop, secondStop), result);

  return result;
}

TransportCatalogueImpl::GetRouteDistance::GetRouteDistance(
    const DistanceMap &routeDistances)
    : routeDistances_(routeDistances) {}

double TransportCatalogueImpl::GetRouteDistance::operator()(
    const StopElement *firstStop, const StopElement *secondStop) const {
  if (routeDistances_.empty()) {
    return 0.;
  }

  if (const auto iter = routeDistances_.find({firstStop, secondStop});
      iter != routeDistances_.end()) {
    return iter->second;
  }
  if (const auto iter = routeDistances_.find({secondStop, firstStop});
      iter != routeDistances_.end()) {
    return iter->second;
  }
  return 0.;
}
