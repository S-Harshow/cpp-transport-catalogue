#include "request_handler.h"

#include <utility>
using namespace transport;
using namespace transport::detail;

void transport::detail::BaseRequestHandler::operator()(
    const StopQuery &query) const {
  detail::Coordinates coordinates{query.latitude, query.longitude};
  std::unordered_map<std::string_view, int> route_distances(
      query.road_distances.begin(), query.road_distances.end());
  db_.addStop(query.name, coordinates, route_distances);
}

void transport::detail::BaseRequestHandler::operator()(
    const BusQuery &query) const {
  std::vector<std::string_view> stops(query.stops.begin(), query.stops.end());
  if (!query.is_roundtrip) {
    int size = static_cast<int>(stops.size());
    stops.resize(2 * stops.size() - 1);
    copy(stops.begin(), stops.begin() + size - 1, stops.begin() + size);
    reverse(stops.begin() + size, stops.end());
  }
  db_.addBus(query.name, stops, query.is_roundtrip);
}

void transport::detail::BaseRequestHandler::operator()(
    const std::monostate &) const {}

transport::Response transport::detail::StatRequestHandler::operator()(
    const StopQuery &query) const {
  auto response = db_.getStopStat(query.name);
  if (response.has_value()) {
    return {response.value(), request_id_};
  }
  return {{}, request_id_};
}

transport::Response
transport::detail::StatRequestHandler::operator()(const BusQuery &query) const {
  auto response = db_.getBusStat(query.name);
  if (response.has_value()) {
    return {response.value(), request_id_};
  }
  return {{}, request_id_};
}

transport::Response
transport::detail::StatRequestHandler::operator()(const MapQuery &query) const {
  auto routes = db_.getRoutesInfo();
  if (routes.has_value()) {
    // Напинать рендер, что б поработал и вернул картинку
    MapStat response_value(query.name);
    auto doc = renderer_.renderRoutesMap(std::move(routes.value()));
    std::ostringstream str_stream;
    doc.Render(str_stream);
    //    std::cout << str_stream.str();
    response_value.map_string = str_stream.str();
    return transport::Response(response_value, request_id_);
  }
  return transport::Response({}, request_id_);
}

transport::Response transport::detail::StatRequestHandler::operator()(
    const std::monostate & /*unused*/) const {
  return {{}, request_id_};
}

transport::RequestHandler::RequestHandler(TransportCatalogue &db,
                                          MapRenderer &renderer)
    : db_(db), renderer_(renderer) {}

void transport::RequestHandler::SetOutputter(
    std::shared_ptr<Outputter> outputter) {
  outputter_ = std::move(outputter);
}

void transport::RequestHandler::SetInputter(
    std::shared_ptr<Inputter> inputter) {
  inputter_ = std::move(inputter);
}

void transport::RequestHandler::ProcessRequests() {
  if (inputter_ == nullptr) {
    return;
  }

  // от inputter получаю список запросов в "промежуточном" формате
  inputter_->ParseInput();

  // Внести новые данные. По очереди передаю их транспортному каталогу
  if (auto requests = inputter_->GetRequests(RequestType::Base);
      requests.has_value()) {
    std::for_each(
        requests->begin(), requests->end(), [*this](const Request &request) {
          return std::visit(detail::BaseRequestHandler{db_}, request.value);
        });
  }
  // Найти и применить настройки рисовальщака катры
  if (auto requests = inputter_->GetRequests(RequestType::Render);
      requests.has_value()) {
    renderer::RenderSettings settings =
        std::get<renderer::RenderSettings>(requests->at(0).value);
    renderer_.SetSettings(settings);
  }

  // Обработать статистические запросы. Если от каталога получен ответ -
  // сохраняю ответ
  Responses result{};
  if (auto requests = inputter_->GetRequests(RequestType::Stat);
      requests.has_value()) {
    std::transform(
        requests->begin(), requests->end(), back_inserter(result),
        [*this](const Request &request) {
          return std::visit(
              detail::StatRequestHandler{db_, renderer_, request.requestId},
              request.value);
        });
  }

  outputter_->SetResponses(result);
}

svg::Document transport::RequestHandler::RenderMap() const {
  // Запросить вектор маршрутов
  auto routes = db_.getRoutesInfo();
  if (routes != std::nullopt) {
    // Напинать рендер, что б поработал и вернул картинку
    return renderer_.renderRoutesMap(std::move(routes.value()));
  }
  return {};
}
