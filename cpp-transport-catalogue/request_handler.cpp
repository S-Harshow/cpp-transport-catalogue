#include "request_handler.h"
#include "json_reader.h"

#include <ostream>
#include <utility>

using namespace std;
using namespace transport;
using namespace transport::detail;

void transport::RequestHandler::Execute() {
  if (inputter_ == nullptr) {
    return;
  }
  // получаю на обработку запросы из входног потока
  inputter_->Parse();
  for (const auto &query : *inputter_) {
    query->Execute(*this);
  }
  // готовый документ отгружаю в выходной поток
  outputter_->Send();
}

void ModifyQuery::Execute(QueryVisitor &visitor) const { Process(visitor); }

void ComputeQuery::Execute(QueryVisitor &visitor) const { Process(visitor); }

namespace transport::queries {

EmptyResponse::EmptyResponse(int requestId) : id_(requestId) {}

void EmptyResponse::accept(Outputter &outputter) { outputter.visit(this); }

int EmptyResponse::getId() const { return id_; }
std::unique_ptr<Response> EmptyResponse::Factory::Construct(int id) const {
  return std::make_unique<EmptyResponse>(id);
}

namespace bus {
AddBusQuery::AddBusQuery(const BusData &data) : data_(data) {}

void AddBusQuery::Process(QueryVisitor &visitor) const {
  if (nullptr != visitor.getCatalog()) {
    visitor.getCatalog()->addBus(data_);
  }
}

AddBusQuery::Factory &AddBusQuery::Factory::SetName(const std::string &name) {
  data_.name = name;
  return *this;
}

AddBusQuery::Factory &
AddBusQuery::Factory::SetStops(const std::vector<std::string> &stops) {
  data_.stops = stops;
  return *this;
}

AddBusQuery::Factory &
AddBusQuery::Factory::SetRoundTripMark(const bool is_roundtrip) {
  data_.is_roundtrip = is_roundtrip;
  return *this;
}

uniqueQuery AddBusQuery::Factory::Construct() const {
  if (data_.name.empty()) {
    throw std::logic_error("Name was not added");
  }
  return std::make_unique<AddBusQuery>(data_);
}

StatBusQuery::StatBusQuery(int requestId, const std::string &name)
    : id_(requestId), name_(name) {}

void StatBusQuery::Process(QueryVisitor &visitor) const {
  if (nullptr != visitor.getCatalog()) {
    auto response = visitor.getCatalog()->getBusStat(name_);
    if (response.has_value()) {
      visitor.appendResponse(
          StatResponse::Factory().SetResponse(response.value()).Construct(id_));
      return;
    }
    visitor.appendResponse(EmptyResponse::Factory().Construct(id_));
  }
}

StatBusQuery::Factory &StatBusQuery::Factory::SetName(const std::string &name) {
  name_ = name;
  return *this;
}

StatBusQuery::Factory &StatBusQuery::Factory::SetId(int requestId) {
  id_ = requestId;
  return *this;
}

uniqueQuery StatBusQuery::Factory::Construct() const {
  return std::unique_ptr<Query>(new StatBusQuery(id_, name_));
}

StatResponse::Factory &StatResponse::Factory::SetResponse(const BusStat &data) {
  data_ = data;
  return *this;
}

std::unique_ptr<Response> StatResponse::Factory::Construct(int id) const {
  return std::make_unique<StatResponse>(id, data_);
}
StatResponse::StatResponse(int id, const BusStat &data)
    : id_(id), data_(data) {}

void StatResponse::accept(Outputter &outputter) { outputter.visit(this); }

int StatResponse::getId() const { return id_; }

string StatResponse::getName() const { return string(data_.name); }

uint StatResponse::getStopsCount() const { return data_.stops_count; }

uint StatResponse::getUniqueStopsCount() const {
  return data_.unique_stops_count;
}

double StatResponse::getGeoLength() const { return data_.geolength; }

double StatResponse::getRouteLength() const { return data_.routelength; }

} // namespace bus
namespace stop {
AddStopQuery::AddStopQuery(const StopData &data) : data_(data) {}

void AddStopQuery::Process(QueryVisitor &visitor) const {
  if (nullptr != visitor.getCatalog()) {
    visitor.getCatalog()->addStop(data_);
  }
}

AddStopQuery::Factory &AddStopQuery::Factory::SetName(const std::string &name) {
  data_.name = name;
  return *this;
}

AddStopQuery::Factory &AddStopQuery::Factory::SetCoordinates(double latitude,
                                                             double longitude) {
  data_.coordinates = {latitude, longitude};
  return *this;
}

AddStopQuery::Factory &AddStopQuery::Factory::SetDistances(
    std::map<std::string, double> road_distances) {
  data_.road_distances.swap(road_distances);
  return *this;
}

uniqueQuery AddStopQuery::Factory::Construct() const {
  if (data_.name.empty()) {
    throw std::logic_error("Name was not added");
  }

  if (data_.coordinates == detail::Coordinates()) {
    throw std::logic_error("Coordinates was not added");
  }
  return std::make_unique<AddStopQuery>(data_);
}

StatStopQuery::StatStopQuery(int requestId, const std::string &name)
    : id_(requestId), name_(name) {}

void StatStopQuery::Process(QueryVisitor &visitor) const {
  //  cout << "Start StatStopQuery::Process  id = " << id_ << endl;
  if (nullptr != visitor.getCatalog()) {
    if (auto response = visitor.getCatalog()->getStopStat(name_);
        response.has_value()) {
      visitor.appendResponse(
          StatResponse::Factory().SetResponse(response.value()).Construct(id_));
      return;
    }
    visitor.appendResponse(EmptyResponse::Factory().Construct(id_));
  }
}

StatStopQuery::Factory &
StatStopQuery::Factory::SetName(const std::string &name) {
  name_ = name;
  return *this;
}

StatStopQuery::Factory &StatStopQuery::Factory::SetId(int requestId) {
  id_ = requestId;
  return *this;
}

uniqueQuery StatStopQuery::Factory::Construct() const {
  if (name_.empty()) {
    throw std::logic_error("Name was not added");
  }
  if (0 == id_) {
    throw std::logic_error("Id was not added");
  }
  return std::make_unique<StatStopQuery>(id_, name_);
}

StatResponse::Factory &
queries::stop::StatResponse::Factory::SetResponse(const StopStat &data) {
  data_ = data;
  return *this;
}

std::unique_ptr<Response> StatResponse::Factory::Construct(int id) const {
  return std::make_unique<StatResponse>(id, data_);
}

StatResponse::StatResponse(int id, const StopStat &data)
    : id_(id), data_(data) {}

int StatResponse::getId() const { return id_; }

string StatResponse::getName() const { return data_.name; }

vector<string>::const_iterator StatResponse::buses_cbegin() const {
  return data_.buses.cbegin();
}

vector<string>::const_iterator StatResponse::buses_cend() const {
  return data_.buses.cend();
}

std::vector<string> StatResponse::getBuses() const { return data_.buses; }

void StatResponse::accept(Outputter &outputter) { outputter.visit(this); }

} // namespace stop

namespace map {
RenderSettings::RenderSettings(const renderer::RenderSettings &settings)
    : settings_(settings) {}

void RenderSettings::Execute(QueryVisitor &visitor) const { Process(visitor); }

void RenderSettings::Process(QueryVisitor &visitor) const {
  //
  if (nullptr != visitor.getRenderer()) {
    visitor.getRenderer()->SetSettings(settings_);
  }
}

RenderSettings::Factory &
RenderSettings::Factory::SetSettings(const renderer::RenderSettings &settings) {
  settings_ = settings;
  return *this;
}

uniqueQuery RenderSettings::Factory::Construct() const {
  if (!settings_) {
    throw std::logic_error("Renderer settings are not valid");
  }
  return std::unique_ptr<Query>(new RenderSettings(settings_));
}

MapRender::Factory &MapRender::Factory::SetId(int requestId) {
  id_ = requestId;
  return *this;
}

uniqueQuery MapRender::Factory::Construct() const {
  return std::make_unique<MapRender>(id_);
}

MapRender::MapRender(int requestId) : id_(requestId) {}

void MapRender::Process(QueryVisitor &visitor) const {
  if (nullptr == visitor.getCatalog() || nullptr == visitor.getRenderer()) {
    visitor.appendResponse(EmptyResponse::Factory().Construct(id_));
  }
  auto routes = visitor.getCatalog()->getRoutesInfo();
  if (routes.has_value()) {
    // Напинать рендер, что б поработал и вернул картинку
    auto doc = visitor.getRenderer()->renderRoutesMap(routes.value());
    std::ostringstream str_stream;
    doc.Render(str_stream);
    //    std::cout << str_stream.str();
    visitor.appendResponse(
        MapResponse::Factory().SetResponse(str_stream.str()).Construct(id_));
    return;
  }
  visitor.appendResponse(EmptyResponse::Factory().Construct(id_));
}

MapResponse::Factory &
MapResponse::Factory::SetResponse(const std::string &data) {
  data_ = data;
  return *this;
}

std::unique_ptr<Response> MapResponse::Factory::Construct(int id) const {
  return std::make_unique<MapResponse>(id, data_);
}

MapResponse::MapResponse(int id, const string &data) : id_(id), data_(data) {}

int MapResponse::getId() const { return id_; }

string MapResponse::getMap() const { return data_; }

void MapResponse::accept(Outputter &outputter) { outputter.visit(this); }

} // namespace map

namespace router {
RoutingSettings::RoutingSettings(
    const transport::router::RoutingSettings &settings)
    : settings_(settings) {}

RoutingSettings::Factory &
RoutingSettings::Factory::SetBusWaitTime(double time) {
  //  if (time - 1 > EPSILON && time - 1000 < EPSILON) {
  settings_.bus_wait = time;
  //  }
  return *this;
}

RoutingSettings::Factory &
RoutingSettings::Factory::SetBusVelocity(double velocity) {
  //  if (velocity - 1 > EPSILON && velocity - 1000 < EPSILON) {
  settings_.bus_velocity = velocity;
  //  }
  return *this;
}

uniqueQuery RoutingSettings::Factory::Construct() const {
  if (settings_.bus_wait < 1. || settings_.bus_velocity < 1.0) {
    throw std::logic_error("Routing settings are not valid");
  }
  return std::unique_ptr<Query>(new RoutingSettings(settings_));
}

void RoutingSettings::Execute(QueryVisitor &visitor) const { Process(visitor); }

void RoutingSettings::Process(QueryVisitor &visitor) const {
  //
  if (visitor.getRouter() != nullptr) {
    visitor.getRouter()->SetSettings(settings_);
  }
}

RouteQuery::Factory &RouteQuery::Factory::SetId(int requestId) {
  id_ = requestId;
  return *this;
}

RouteQuery::Factory &RouteQuery::Factory::SetFromStop(const std::string &from) {
  stop_from_ = from;
  return *this;
}

RouteQuery::Factory &RouteQuery::Factory::SetToStop(const std::string &to) {
  stop_to_ = to;
  return *this;
}

uniqueQuery RouteQuery::Factory::Construct() const {
  if (stop_from_.empty()) {
    throw std::logic_error("No stop \"from\" was found");
  }
  if (stop_to_.empty()) {
    throw std::logic_error("No stop \"to\" was found");
  }
  return std::unique_ptr<Query>(new RouteQuery(id_, stop_from_, stop_to_));
}

RouteQuery::RouteQuery(int id, const std::string &stop_from,
                       const std::string &stop_to)
    : id_(id), stop_from_(stop_from), stop_to_(stop_to) {}

void RouteQuery::Process(QueryVisitor &visitor) const {
  //  if (nullptr == visitor.getCatalog() || nullptr == visitor.getRouter()) {
  //    visitor.appendResponse(EmptyResponse::Factory().Construct(id_));
  //  }
  if (!visitor.getRouter()->IsReady()) {
    const auto routes(visitor.getCatalog()->getRoutesInfo());
    const auto distances(visitor.getCatalog()->getDistances());
    const size_t stops_count(visitor.getCatalog()->getStopCount());
    if (routes.has_value()) {
      visitor.getRouter()->UploadData(stops_count, routes.value(), distances);
    }
  }
  if (visitor.getRouter()->IsReady()) {
    auto result(visitor.getRouter()->FindRoute(stop_from_, stop_to_));
    if (result.has_value()) {
      visitor.appendResponse(
          RouteResponse::Factory().SetResponse(result.value()).Construct(id_));
      return;
    }
  }
  visitor.appendResponse(EmptyResponse::Factory().Construct(id_));
}

RouteResponse::RouteResponse(int id, const RouteStat &data)
    : id_(id), data_(data) {}

void RouteResponse::accept(Outputter &outputter) { outputter.visit(this); }

int RouteResponse::getId() const { return id_; }

double RouteResponse::getTotalTime() const { return data_.total_time; }

size_t RouteResponse::items_size() const { return data_.items.size(); }

vector<RouteItem>::const_iterator RouteResponse::items_cbegin() const {
  return data_.items.cbegin();
}

vector<RouteItem>::const_iterator RouteResponse::items_cend() const {
  return data_.items.cend();
}

RouteResponse::Factory &
RouteResponse::Factory::SetResponse(const RouteStat &data) {
  data_ = data;
  return *this;
}

std::unique_ptr<Response> RouteResponse::Factory::Construct(int id) const {
  return std::make_unique<RouteResponse>(id, data_);
}

} // namespace router

// namespace router
} // namespace transport::queries

RequestHandler::RequestHandler(std::unique_ptr<Inputter> inputter,
                               std::unique_ptr<Outputter> outputter)
    : inputter_(move(inputter)), outputter_(move(outputter)),
      db_(TransportCatalogue::Make()), renderer_(MapRenderer::Make()),
      router_(TransportRouter::Make()) {}

TransportCatalogue *RequestHandler::getCatalog() const { return db_.get(); }

MapRenderer *RequestHandler::getRenderer() const { return renderer_.get(); }

TransportRouter *RequestHandler::getRouter() const { return router_.get(); }

void RequestHandler::appendResponse(uniqueResponse response) {
  response->accept(*outputter_);
}
