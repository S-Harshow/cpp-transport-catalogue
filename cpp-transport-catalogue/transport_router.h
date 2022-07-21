#pragma once
#include "graph.h"
#include "router.h"
#include "domain.h"
#include <memory>

namespace transport {

namespace router {
struct RoutingSettings {
  double bus_wait{};
  double bus_velocity{};
};

} // namespace router

class TransportRouter {
public:
  virtual ~TransportRouter() = default;
  static std::unique_ptr<TransportRouter> Make();

  virtual void SetSettings(const router::RoutingSettings &) = 0;
  virtual void UploadData(size_t, const std::vector<BusInfo> &,
                          const Distances &) = 0;
  [[nodiscard]] virtual bool IsReady() = 0;
  [[nodiscard]] virtual std::optional<RouteStat>
  FindRoute(const std::string &stop_from, const std::string &stop_to) const = 0;
};

class TransportRouterImpl : public TransportRouter {
public:
  void SetSettings(const router::RoutingSettings & /*unused*/) override;
  [[nodiscard]] bool IsReady() override;
  void UploadData(size_t stops_count, const std::vector<BusInfo> & /*unused*/,
                  const Distances &distances) override;
  [[nodiscard]] std::optional<RouteStat>
  FindRoute(const std::string &stop_from,
            const std::string &stop_to) const override;

private:
  struct InternalEdge {
    InternalEdge(std::string_view from, uint span_count, double time,
                 std::string_view bus_name)
        : from(from), span_count(span_count), time(time), bus_name(bus_name) {}
    std::string_view from;
    uint span_count;
    double time;
    std::string_view bus_name;
  };

  router::RoutingSettings settings_;

  graph::VertexId vertex_counter_{};
  std::unordered_map<std::string_view, graph::VertexId> vertex_index_{};

  std::unordered_map<graph::EdgeId, InternalEdge> edge_index_{};

  std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
  std::unique_ptr<graph::Router<double>> router_;

  graph::VertexId GetVertexIDByName(std::string_view stop_name);

  //  void FillGraph(const BusInfo &bus_info, const Distances &distances);

  template <typename Iterator>
  void FillGraph(std::string_view bus_name, Iterator begin, Iterator end,
                 const Distances &distances);
};

} // namespace transport
