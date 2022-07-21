#pragma once

#include "../../src/request_handler.h"

namespace transport {
class MockTransportCatalogue : public TransportCatalogue {
public:
  MockTransportCatalogue() = default;

  void addBus(const BusData &) override;

  void addStop(const StopData &) override;

  std::optional<BusElement *> findBus(const std::string &) const override;

  std::optional<BusStat> getBusStat(const std::string &) const override;

  std::optional<StopStat> getStopStat(const std::string &) const override;

  std::optional<std::vector<BusInfo>> getRoutesInfo() const override;

  Distances getDistances() const override;

  size_t getStopCount() const override;


  std::vector<BusData> add_bus_requests{};
  std::vector<StopData> add_Stop_requests{};
  mutable std::string bus_to_find{};
  mutable std::string stop_to_find{};
  mutable std::string bus_to_stat{};
  mutable std::string stop_to_stat{};
};

class MockRenderer : public MapRenderer {
  // MapRenderer interface
public:
  void SetSettings(const renderer::RenderSettings &) override;
  bool hasSettings() override;
  svg::Document renderRoutesMap(const std::vector<BusInfo> &) const override;

  renderer::RenderSettings settings_;
  mutable std::vector<BusInfo> buses_for_map{};
};

class MockRouter :public TransportRouter {
public:
  void SetSettings(const router::RoutingSettings &) override;
//  [[nodiscard]] bool HasSettings() override;
  void UploadData(size_t stops_count, const std::vector<BusInfo> &, const Distances &dist) override;

  router::RoutingSettings settings;

  // TransportRouter interface
public:
  bool IsReady() override;
  std::optional<RouteStat> FindRoute(const std::string& stop_from, const std::string& stop_to) const override;
};

class MockHandler : public QueryVisitor {
public:
  MockHandler(std::unique_ptr<Inputter> inputter,
              std::unique_ptr<Outputter> outputter,
              std::shared_ptr<MockTransportCatalogue> db,
              std::shared_ptr<MockRenderer> renderer,
              std::shared_ptr<MockRouter> router);

  void Execute();

  [[nodiscard]] svg::Document RenderMap() const;
  [[nodiscard]] TransportCatalogue *getCatalog() const override;
  [[nodiscard]] MapRenderer *getRenderer() const override;
  [[nodiscard]] TransportRouter *getRouter() const override;
  void appendResponse(uniqueResponse response) override;

  void ParseInput();
  void SendOutput();
  [[nodiscard]] uniqueQueryList::const_iterator Inputter_cbegin() const ;
  [[nodiscard]] uniqueQueryList::iterator Inputter_begin();
  [[nodiscard]] uniqueQueryList::const_iterator Inputter_cend() const ;
  [[nodiscard]] uniqueQueryList::iterator Inputter_end();

private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и
  // "Визуализатор Карты"
  std::unique_ptr<Inputter> inputter_;
  std::unique_ptr<Outputter> outputter_;
  std::shared_ptr<MockTransportCatalogue> db_;
  std::shared_ptr<MockRenderer> renderer_;
  std::shared_ptr<MockRouter> router_;
};

} // namespace transport
