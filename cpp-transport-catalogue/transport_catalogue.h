#pragma once

#include "geo.h"
#include <cstddef>
#include <deque>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std::string_literals;

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

namespace transport {
class TransportCatalogue {
public:
  struct Bus;
  struct Stop {
    Stop() = default;
    explicit Stop(const std::string &name) : name(name){};
    explicit Stop(const std::string &name, const detail::Coordinates coordinates)
        : name(name), coordinates(coordinates){};
    const std::string name{};
    detail::Coordinates coordinates{};
    // набор автобусов по остановке, set для избавления от повторов
    std::set<Bus *> buses{};
  };

  struct Bus {
    Bus() = default;
    explicit Bus(const std::string &name, const std::vector<Stop *> &stops)
        : name(name), stops(stops){};
    const std::string name{};
    // вектор с остановками по маршруту автобуса, vector - важен порядок
    const std::vector<Stop *> stops{};
  };

  struct StopInfo {
    StopInfo() = default;
    explicit StopInfo(const std::string &name) : name(name){};
    const std::string name{};
    std::vector<std::string_view> buses{};
  };

  struct BusInfo {
    BusInfo() = default;
    explicit BusInfo(std::string_view name) : name(std::string(name)){};
    const std::string name{};
    size_t stops = 0;
    size_t unique_stops = 0;
    double geolength = .0;
    double routelength = .0;
  };
  // алиасы
  using StopPtrPair = std::pair<const Stop *const, const Stop *const>;
  using DistanceMap =
      std::unordered_map<StopPtrPair, double, detail::pair_hash>;

  explicit TransportCatalogue() = default;
  // добавление маршрута автобуса в базу
  // Если остановка не внесена в базу - заносит
  void addBus(std::string_view, const std::vector<std::string_view> &);
  size_t getBusesCount() const;
  //  добавление остановки в базу и её описания
  void addStop(std::string_view, detail::Coordinates,
               std::unordered_map<std::string_view, unsigned long> &);
  size_t getStopsCount() const;
  //  поиск маршрута по имени
  std::optional<Bus *> findBus(std::string_view) const;
  //  поиск остановки по имени,
  std::optional<Stop *> findStop(std::string_view) const;
  //  получение информации о маршруте.
  std::optional<BusInfo> getBusInfo(std::string_view) const;
  //  получение информации об остановке.
  std::optional<StopInfo> getStopInfo(std::string_view) const;

private:
  // контейнер остановок
  std::deque<Stop> stops_;
  // индекс остановок
  std::unordered_map<std::string_view, Stop *const> stopsIndex_;
  // контейнер маршрутов
  std::deque<Bus> buses_;
  // индекс маршрутов
  std::unordered_map<std::string_view, Bus *const> busesIndex_;
  // расстояния географические (по координатам)
  mutable DistanceMap geoDistances_;
  // расстояния измеренные (по одометру)
  DistanceMap routeDistances_;

  struct GetKnownStop {
    GetKnownStop(std::vector<TransportCatalogue::Stop *> &busStops,
                 TransportCatalogue *const catalog)
        : busStops_(busStops), catalog_(catalog){};
    void operator()(std::string_view stop_name) const {
      auto stop = catalog_->findStop(std::string(stop_name));
      if (!stop) { // если не нашлась - создает новую
        stop = std::optional<TransportCatalogue::Stop *>{
            catalog_->addStop_(stop_name)};
      }
      busStops_.push_back(*stop);
    }

  private:
    std::vector<TransportCatalogue::Stop *> &busStops_;
    TransportCatalogue *const catalog_;
  };
  struct GetGeoDistance {
    explicit GetGeoDistance(TransportCatalogue::DistanceMap &distances)
        : distances_(distances) {}
    double operator()(const TransportCatalogue::Stop *firstStop,
                      const TransportCatalogue::Stop *secondStop) const {
      double result = .0;
      const auto iter = distances_.find({firstStop, secondStop});
      if (iter == distances_.end()) {
        // рассчитать расстояние
        result =
            ComputeDistance(firstStop->coordinates, secondStop->coordinates);
        distances_.emplace(std::make_pair(firstStop, secondStop), result);

      } else {
        result = iter->second;
      }
      return result;
    }

  private:
    TransportCatalogue::DistanceMap &distances_;
  };
  struct GetDistance {
    explicit GetDistance(const TransportCatalogue::DistanceMap &routeDistances)
        : routeDistances_(routeDistances) {}
    unsigned long operator()(const TransportCatalogue::Stop *firstStop,
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

  private:
    const TransportCatalogue::DistanceMap &routeDistances_;
  };
  Stop *addStop_(std::string_view);
};
} // namespace transport
