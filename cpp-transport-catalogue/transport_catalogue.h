#pragma once

/*
 * Здесь можно разместить код транспортного справочника
 */

#include "domain.h"
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

namespace transport {
class TransportCatalogue {
public:
  // алиасы
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
    bool is_roundtrip{};
  };
  using StopPtrPair = std::pair<const Stop *const, const Stop *const>;
  using DistanceMap =
      std::unordered_map<StopPtrPair, double, detail::pair_hash>;
  explicit TransportCatalogue() = default;
  // добавление маршрута автобуса в базу производится со сложностью
  // амортизированная O(K) в среднем, где K — длина названия
  void addBus(std::string_view, const std::vector<std::string_view> &, bool);
  // reduce
  size_t getBusesCount() const;
  // добавление остановки в базу и её описания производится со сложностью
  // амортизированная O(K) в среднем, где K — длина названия
  void addStop(std::string_view, detail::Coordinates,
               std::unordered_map<std::string_view, int> &);
  // reduce
  size_t getStopsCount() const;
  // нахождение маршрута по названию в среднем амортизированная O(K), где K —
  // длина названия
  std::optional<Bus *> findBus(std::string_view) const;
  // нахождение остановки по названию в среднем амортизированная O(K), где K —
  // длина названия
  std::optional<Stop *> findStop(std::string_view) const;
  // получение статистической информации о маршруте производится со сложностью
  // амортизированная O(1) в среднем.
  std::optional<BusStat> getBusStat(std::string_view) const;
  //  получение статистической информации об остановке. Требований по сложности
  //  не установленно
  std::optional<StopStat> getStopStat(std::string_view) const;
  // получение информации о всех маршрутах
  std::optional<std::vector<BusInfo>> getRoutesInfo() const;

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
    GetKnownStop(std::vector<Stop *> &, TransportCatalogue *);
    void operator()(std::string_view) const;

  private:
    std::vector<Stop *> &busStops_;
    TransportCatalogue *const catalog_;
  };
  struct GetGeoDistance {
    explicit GetGeoDistance(TransportCatalogue::DistanceMap &distances);
    double operator()(const Stop *firstStop, const Stop *secondStop) const;

  private:
    TransportCatalogue::DistanceMap &distances_;
  };
  struct GetDistance {
    explicit GetDistance(const TransportCatalogue::DistanceMap &routeDistances);
    unsigned long operator()(const Stop *firstStop,
                             const Stop *secondStop) const;

  private:
    const TransportCatalogue::DistanceMap &routeDistances_;
  };
  Stop *addStop_(std::string_view);
};
} // namespace transport
