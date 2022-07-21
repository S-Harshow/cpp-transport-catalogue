#include <utility>

#include "fakehandler.h"

using namespace std;

void MockRenderer::SetSettings(const renderer::RenderSettings &settings) {
  settings_ = settings;
}

bool MockRenderer::hasSettings() { return settings_.isValid(); }

svg::Document
MockRenderer::renderRoutesMap(const std::vector<BusInfo> &buses) const {
  buses_for_map = buses;
  return {};
}

MockHandler::MockHandler(std::unique_ptr<Inputter> inputter,
                         std::unique_ptr<Outputter> outputter,
                         std::shared_ptr<MockTransportCatalogue> db,
                         std::shared_ptr<MockRenderer> renderer,
                         std::shared_ptr<MockRouter> router)
    : inputter_(move(inputter)), outputter_(move(outputter)),
      db_(std::move(db)), renderer_(std::move(renderer)),
      router_(std::move(router)) {}

void MockHandler::Execute() {

  inputter_->Parse();
  for (const auto &query : *inputter_) {
    query->Execute(*this);
  }
}

svg::Document MockHandler::RenderMap() const { return {}; }

TransportCatalogue *MockHandler::getCatalog() const { return db_.get(); }

MapRenderer *MockHandler::getRenderer() const { return renderer_.get(); }

TransportRouter *MockHandler::getRouter() const { return router_.get(); }

void MockHandler::appendResponse(uniqueResponse response) {
  response->accept(*outputter_);
}

void MockHandler::ParseInput() { inputter_->Parse(); }

void MockHandler::SendOutput() { outputter_->Send(); }

uniqueQueryList::const_iterator MockHandler::Inputter_cbegin() const {
  return inputter_->cbegin();
}

uniqueQueryList::iterator MockHandler::Inputter_begin() {
  return inputter_->begin();
}

uniqueQueryList::const_iterator MockHandler::Inputter_cend() const {
  return inputter_->cend();
}

uniqueQueryList::iterator MockHandler::Inputter_end() {
  return inputter_->end();
}

void MockTransportCatalogue::addBus(const BusData &new_bus) {
  add_bus_requests.push_back(new_bus);
}

void MockTransportCatalogue::addStop(const StopData &new_stop) {
  add_Stop_requests.push_back(new_stop);
}

std::optional<TransportCatalogue::BusElement *>
MockTransportCatalogue::findBus(const std::string &bus_name) const {
  bus_to_find = std::string(bus_name);
  return {};
}

std::optional<BusStat>
MockTransportCatalogue::getBusStat(const string &bus_name) const {
  bus_to_stat = std::string(bus_name);
  return {};
}

std::optional<StopStat>
MockTransportCatalogue::getStopStat(const std::string &stop_name) const {
  stop_to_stat = std::string(stop_name);
  return {};
}

std::optional<std::vector<BusInfo>>
MockTransportCatalogue::getRoutesInfo() const {
  const double one_lat = 43.587795;
  const double one_lng = 39.716901;
  const double two_lat = 43.581969;
  const double two_lng = 39.719848;
  std::vector<BusInfo> result;
  BusInfo element("114");
  element.is_roundtrip = false;
  element.stops.reserve(2);
  element.stops.push_back({"Ривьерский мост", {one_lat, one_lng}});
  element.stops.push_back({"Морской вокзал", {two_lat, two_lng}});

  result.push_back(element);
  return result;
}

Distances MockTransportCatalogue::getDistances() const { return {}; }

size_t MockTransportCatalogue::getStopCount() const { return 100; }

void MockRouter::SetSettings(const router::RoutingSettings &new_settings) {
  settings = new_settings;
}

void MockRouter::UploadData(size_t stops_count,
                            const std::vector<BusInfo> &bus_info,
                            const Distances &dist) {
  (void)stops_count;
  (void)bus_info;
  (void)dist;
}

bool MockRouter::IsReady() { return false; }

std::optional<RouteStat> MockRouter::FindRoute(const string &stop_from,
                                               const string &stop_to) const {
  (void)stop_from;
  (void)stop_to;
  //
  return nullopt;
}
