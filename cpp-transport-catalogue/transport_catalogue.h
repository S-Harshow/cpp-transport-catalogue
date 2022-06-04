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
    explicit Stop(const std::string &);
    explicit Stop(const std::string &, detail::Coordinates);
    const std::string name{};
    std::optional<detail::Coordinates> coordinates{};
    // набор автобусов по остановке, set для избавления от повторов
    //    std::set<Bus *> buses{};
    std::set<std::string_view> buses{};
  };

  struct Bus {
    Bus() = default;
    explicit Bus(const std::string &, const std::vector<Stop *> &);
    const std::string name{};
    // вектор с остановками по маршруту автобуса, vector - важен порядок
    const std::vector<Stop *> stops{};
    double routelength = .0;
  };

  struct StopInfo {
    StopInfo() = default;
    explicit StopInfo(const std::string &);
    explicit StopInfo(const std::string &,
                      const std::vector<std::string_view> &);
    const std::string name{};
    std::vector<std::string_view> buses{};
  };

  struct BusInfo {
    BusInfo() = default;
    explicit BusInfo(std::string_view);
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
  // добавление маршрута автобуса в базу производится со сложностью
  // амортизированная O(K) в среднем, где K — длина названия
  void addBus(std::string_view, const std::vector<std::string_view> &);
  // reduce
  size_t getBusesCount() const;
  // добавление остановки в базу и её описания производится со сложностью
  // амортизированная O(K) в среднем, где K — длина названия
  void addStop(std::string_view, detail::Coordinates,
               std::unordered_map<std::string_view, unsigned long> &);
  // reduce
  size_t getStopsCount() const;
  // нахождение маршрута по названию в среднем амортизированная O(K), где K —
  // длина названия
  std::optional<Bus *> findBus(std::string_view) const;
  // нахождение остановки по названию в среднем амортизированная O(K), где K —
  // длина названия
  std::optional<Stop *> findStop(std::string_view) const;
  // получение информации о маршруте производится со сложностью амортизированная
  // O(1) в среднем.
  std::optional<BusInfo> getBusInfo(std::string_view) const;
  //  получение информации об остановке. Требований по сложности не установленно
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
    GetKnownStop(std::vector<TransportCatalogue::Stop *> &,
                 TransportCatalogue *);
    void operator()(std::string_view) const;

  private:
    std::vector<TransportCatalogue::Stop *> &busStops_;
    TransportCatalogue *const catalog_;
  };
  struct GetGeoDistance {
    explicit GetGeoDistance(TransportCatalogue::DistanceMap &distances);
    double operator()(const TransportCatalogue::Stop *firstStop,
                      const TransportCatalogue::Stop *secondStop) const;

  private:
    TransportCatalogue::DistanceMap &distances_;
  };
  struct GetDistance {
    explicit GetDistance(const TransportCatalogue::DistanceMap &routeDistances);
    unsigned long operator()(const TransportCatalogue::Stop *firstStop,
                             const TransportCatalogue::Stop *secondStop) const;

  private:
    const TransportCatalogue::DistanceMap &routeDistances_;
  };
  Stop *addStop_(std::string_view);
};
} // namespace transport
