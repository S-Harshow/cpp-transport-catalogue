#include "transport_router.h"
#include <execution>
#include <numeric>

using namespace std;
using namespace transport;
using namespace router;
using namespace graph;

const double VELOCITY_CORRECTION = 0.06;

unique_ptr<TransportRouter> TransportRouter::Make() {
  return std::make_unique<TransportRouterImpl>();
}

void TransportRouterImpl::SetSettings(
    const transport::router::RoutingSettings &settings) {
  settings_ = settings;
}

bool TransportRouterImpl::IsReady() { return !edge_index_.empty(); }

void TransportRouterImpl::UploadData(size_t stops_count,
                                     const vector<BusInfo> &buses_info,
                                     const Distances &distances) {
  vertex_index_.clear();
  edge_index_.clear();

  vertex_index_.reserve(stops_count);
  edge_index_.reserve(distances.size());

  graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(stops_count);

  for_each(buses_info.begin(), buses_info.end(), [&](const BusInfo &bus_info) {
    FillGraph<std::vector<StopInfo>::const_iterator>(
        bus_info.name, bus_info.stops.begin(), bus_info.stops.end(), distances);
    if (!bus_info.is_roundtrip) {
      FillGraph<std::vector<StopInfo>::const_reverse_iterator>(
          bus_info.name, bus_info.stops.rbegin(), bus_info.stops.rend(),
          distances);
    }
  });

  router_ = std::make_unique<graph::Router<double>>(*graph_);
}

std::optional<RouteStat>
TransportRouterImpl::FindRoute(const std::string &stop_from,
                               const std::string &stop_to) const {

  const auto from_iter = vertex_index_.find(stop_from);
  const auto to_iter = vertex_index_.find(stop_to);
  if (from_iter == vertex_index_.end() || to_iter == vertex_index_.end()) {
    return nullopt;
  }
  const auto route_found(
      router_->BuildRoute(from_iter->second, to_iter->second));

  if (route_found.has_value()) {
    RouteStat result{};
    result.total_time = route_found->weight;
    result.items.reserve(route_found->edges.size() * 2);

    for (const EdgeId edge_id : route_found->edges) {
      if (const auto edge_iter = edge_index_.find(edge_id);
          edge_iter != edge_index_.end()) {
        const RouteItemStop stop_item{edge_iter->second.from,
                                      settings_.bus_wait};
        const RouteItemBus bus_item{
            edge_iter->second.bus_name, edge_iter->second.span_count,
            edge_iter->second.time - stop_item.wait_time};
        result.items.emplace_back(stop_item);
        result.items.emplace_back(bus_item);
      }
    }
    return result;
  }
  return nullopt;
}

VertexId TransportRouterImpl::GetVertexIDByName(std::string_view stop_name) {
  // Наполнение индекса номеров остановок (вершин)
  if (const auto iter = vertex_index_.find(stop_name);
      iter != vertex_index_.end()) {
    return iter->second;
  }
  vertex_index_.emplace(stop_name, vertex_counter_);
  return vertex_counter_++;
}

template <typename Iterator>
void TransportRouterImpl::FillGraph(std::string_view bus_name, Iterator begin,
                                    Iterator end, const Distances &distances) {
  Distances loc_distances(distances);
  const double inverse_velocity = VELOCITY_CORRECTION / settings_.bus_velocity;
  std::string prev_name{};
  VertexId from_vertex_id{};
  for (auto iter_from = begin; iter_from < prev(end); ++iter_from) {
    double distance_cumulative{};
    prev_name = iter_from->name;
    from_vertex_id = GetVertexIDByName(iter_from->name);
    for (auto iter_to = next(iter_from); iter_to < end; ++iter_to) {
      const double distance = loc_distances[{prev_name, iter_to->name}];
      if (distance > 0) {
        distance_cumulative += distance;
      } else {
        distance_cumulative += loc_distances[{iter_to->name, prev_name}];
      }
      prev_name = iter_to->name;
      const graph::Edge<double> edge{
          from_vertex_id, GetVertexIDByName(iter_to->name),
          inverse_velocity * distance_cumulative + settings_.bus_wait};
      const EdgeId edge_id = graph_->AddEdge(edge);
      edge_index_.emplace(
          edge_id, InternalEdge(iter_from->name,
                                static_cast<uint>(
                                    abs(std::distance(iter_to, iter_from))),
                                edge.weight, bus_name));
    }
  }
}
