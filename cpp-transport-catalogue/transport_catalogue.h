#pragma once

#include "domain.h"
#include "geo.h"
#include <iomanip>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace std::string_literals;

using namespace transport;

class TransportCatalogue {
protected:
  struct StopElement {
    StopElement() = default;
    explicit StopElement(const std::string & name);
    explicit StopElement(const std::string & , detail::Coordinates);
    const std::string name{};
    std::optional<detail::Coordinates> coordinates{};
    std::set<std::string> buses{};
  };

  struct BusElement {
    BusElement() = default;
    explicit BusElement(const std::string & name,
                        const std::vector<StopElement *> &);
    const std::string name{};
    // вектор с остановками по маршруту автобуса, vector - важен порядок
    const std::vector<StopElement *> stops{};
    unsigned long routelength = 0;
    bool is_roundtrip{};
  };

public:
  virtual ~TransportCatalogue() = default;

  virtual void addBus(const BusData &) = 0;

  virtual void addStop(const StopData &) = 0;

  virtual std::optional<BusElement *> findBus(const std::string &) const = 0;

//  virtual std::optional<StopElement *> findStop(std::string_view) const = 0;

  virtual std::optional<BusStat> getBusStat(const std::string &) const = 0;

  virtual std::optional<StopStat> getStopStat(const std::string &) const = 0;

  virtual std::optional<std::vector<BusInfo>> getRoutesInfo() const = 0;

  virtual Distances getDistances() const = 0;

  virtual size_t getStopCount() const = 0;

  static std::unique_ptr<TransportCatalogue> Make();
};

class TransportCatalogueImpl : public TransportCatalogue {
public:
  // алиасы
  using StopPtrPair =
      std::pair<const StopElement *const, const StopElement *const>;
  using DistanceMap =
      std::unordered_map<StopPtrPair, double, detail::pair_hash>;

  explicit TransportCatalogueImpl() = default;

  void addBus(const BusData &) override;

  void addStop(const StopData &) override;

  std::optional<BusElement *> findBus(const std::string &) const override;

//  std::optional<StopElement *> findStop(std::string_view) const override;

  std::optional<BusStat> getBusStat(const std::string &) const override;

  std::optional<StopStat> getStopStat(const std::string &) const override;

  std::optional<std::vector<BusInfo>> getRoutesInfo() const override;

  Distances getDistances() const override;

  size_t getStopCount() const override;

private:
  // контейнер остановок
  std::deque<StopElement> stops_;
  // индекс остановок по названию
  std::unordered_map<std::string_view, StopElement *const> stops_index_;
  // контейнер маршрутов
  std::deque<BusElement> buses_;
  // индекс маршрутов по названию
  std::unordered_map<std::string_view, BusElement *const> busesIndex_;
  // расстояния вычисленные (географические по координатам)
  mutable DistanceMap geoDistances_;
  // расстояния измеренные (по одометру)
  DistanceMap routeDistances_;

  StopElement *getStop_(std::string_view);

  // функторы

  struct GetGeoDistance {
    explicit GetGeoDistance(DistanceMap &distances);
    double operator()(const StopElement *firstStop,
                      const StopElement *secondStop) const;

  private:
    DistanceMap &distances_;
  };

  struct GetRouteDistance {
    explicit GetRouteDistance(
        const DistanceMap &routeDistances);
    double operator()(const StopElement *firstStop,
                      const StopElement *secondStop) const;

  private:
    const DistanceMap &routeDistances_;
  };
};
