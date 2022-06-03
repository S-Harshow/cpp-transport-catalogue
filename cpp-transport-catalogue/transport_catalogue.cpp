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
void TransportCatalogue::addBus(std::string_view busName,
                                const vector<string_view> &stops) {

  vector<Stop *> bus_stops;
  GetKnownStop getKnownStop(bus_stops, this);
  for_each(stops.begin(), stops.end(), getKnownStop);

  buses_.emplace_back(string(busName), bus_stops);
  Bus &new_bus = buses_.back();
  for_each(new_bus.stops.begin(), new_bus.stops.end(),
           [&new_bus](Stop *stop) { stop->buses.emplace(&new_bus); });
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
  (*stop).coordinates = coordinates;
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
    transform(stops.begin(), prev(stops.end()), next(stops.begin()),
              back_inserter(geoDistances), getGeoDistance);
    vector<unsigned long> distances{};
    GetDistance getDistance(routeDistances_);
    transform(stops.begin(), prev(stops.end()), next(stops.begin()),
              back_inserter(distances), getDistance);

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
  StopInfo stop_info((string(stop_name)));
  auto stop = findStop(stop_name);
  if (stop) {
    for_each(
        (*stop)->buses.begin(), (*stop)->buses.end(),
        [&stop_info](const Bus *bus) { stop_info.buses.push_back(bus->name); });
    sort(stop_info.buses.begin(), stop_info.buses.end());
    return stop_info;
  };
  return {};
}
} // namespace transport
