#pragma once

#include "map_renderer.h"
#include "ranges.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <list>
#include <memory>
#include <unordered_set>

namespace transport {
/* Класс RequestHandler организует совместную работу TransportCatalog, Renderer
 * и Router для обработки запросов, получаемых от Inputter, и передачи ответов в
 * Outputter. Для каждого вида запроса реализован свой класс, который "понимает"
 * как обработать этот запрос. В пространстве имен queries все классы разбиты по
 * пространствам имен bus, stop, map, router. В каждом должны быть определены
 * классы запросов и, при необходимости, ответов.
 */

namespace queries {
class EmptyResponse;
namespace bus {
class StatResponse;
} // namespace bus
namespace stop {
class StatResponse;
} // namespace stop
namespace map {
class MapResponse;
} // namespace map
namespace router {
class RouteResponse;
} // namespace router
} // namespace queries

// Базовый абстрактный класс Outputter - интерфейс для классов, реализующих
// вывод данных в различных форматах. Наследники должны response, получаемый
// через метод visit сохранять свой формат.
class Outputter {
public:
  using IOClass = std::ostream;
  constexpr static const char *const type = "Outputter";

  Outputter() = default;
  Outputter(const Outputter &other) = delete;
  Outputter(Outputter &&other) = delete;
  Outputter &operator=(const Outputter &other) = delete;
  Outputter &operator=(Outputter &&other) = delete;
  virtual ~Outputter() = default;

  virtual void Send() = 0;
  virtual void visit(queries::EmptyResponse *response) = 0;
  virtual void visit(queries::bus::StatResponse *response) = 0;
  virtual void visit(queries::stop::StatResponse *response) = 0;
  virtual void visit(queries::map::MapResponse *response) = 0;
  virtual void visit(queries::router::RouteResponse *response) = 0;
};

// Шаблон сингтона фабрики Inputter/Outputter
template <typename IOPutter> class IOFactory {
public:
  static IOFactory &instance() {
    static IOFactory inst;
    return inst;
  }

  using IOClass = typename IOPutter::IOClass;
  using Factory = std::unique_ptr<IOPutter> (*)(IOClass &);

  void Register(std::string_view name, Factory inputter) {
    factories_.emplace(name, inputter);
  }

  std::unique_ptr<IOPutter> Make(std::string_view name, IOClass &stream) {
    if (factories_.find(name) == factories_.end()) {
      throw std::logic_error("Requested "s + IOPutter::type + " \""s +
                             std::string(name) + "\" is not registered."s);
    }
    Factory factory = factories_.at(name);
    return factory(stream);
  }

private:
  IOFactory() = default;

  std::unordered_map<std::string_view, Factory> factories_{};
};

// Интерфейс результатов выполнения запросов
class Response {
public:
  virtual ~Response() = default;

  // accept используется Outputterом для составления выгружаемого документа
  virtual void accept(Outputter &outputter) = 0;
};

using SharedTransportCatalogue = std::shared_ptr<TransportCatalogue>;
using SharedMapRenderer = std::shared_ptr<MapRenderer>;
using SharedRouter = std::shared_ptr<TransportRouter>;

using uniqueResponse = std::unique_ptr<Response>;
using uniqueResponseList = std::list<uniqueResponse>;

class ResponseFactory {
public:
  ResponseFactory() = default;
  [[nodiscard]] virtual uniqueResponse Construct(int id) const = 0;
  virtual ~ResponseFactory() = default;
};

// Абстрактный базовый класс посещающий запросы и предоставляющий им ресурсы для
// выполнения запросов
class QueryVisitor {
public:
  [[nodiscard]] virtual TransportCatalogue *getCatalog() const = 0;
  [[nodiscard]] virtual MapRenderer *getRenderer() const = 0;
  [[nodiscard]] virtual TransportRouter *getRouter() const = 0;
  virtual void appendResponse(uniqueResponse) = 0;
  virtual ~QueryVisitor() = default;
};

// Абстрактный базовый класс запроса.
// Реализации хранят данные запроса и умеют его выполнять используя
// переданные посетителем рессурсы
class Query {
public:
  virtual ~Query() = default;
  virtual void Execute(QueryVisitor &visitor) const = 0;
};

using uniqueQuery = std::unique_ptr<Query>;
using uniqueQueryList = std::list<uniqueQuery>;

class QueryFactory {
public:
  QueryFactory() = default;
  [[nodiscard]] virtual std::unique_ptr<Query> Construct() const = 0;
  virtual ~QueryFactory() = default;
};

// Классы ModifyQuery и ComputeQuery
class ModifyQuery : public Query {
public:
  using Query::Query;
  //    ModifyQuery();
  ~ModifyQuery() override = default;
  void Execute(QueryVisitor &visitor) const override;

protected:
  virtual void Process(QueryVisitor &visitor) const = 0;
};

class ComputeQuery : public Query {
public:
  using Query::Query;
  //  ComputeQuery();
  ~ComputeQuery() override = default;
  void Execute(QueryVisitor &visitor) const override;

protected:
  virtual void Process(QueryVisitor &visitor) const = 0;
};

namespace queries {

class EmptyResponse final : public Response {
public:
  using Response::Response;
  EmptyResponse(int requestId);
  void accept(Outputter &outputter) override;
  [[nodiscard]] int getId() const;
  class Factory : public ResponseFactory {
  public:
    using ResponseFactory::ResponseFactory;
    [[nodiscard]] std::unique_ptr<Response> Construct(int id) const override;
  };

private:
  int id_{};
};

namespace bus {

class AddBusQuery final : public ModifyQuery {
public:
  using ModifyQuery::ModifyQuery;
  AddBusQuery(const BusData &);
  ~AddBusQuery() override = default;

  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetName(const std::string &name);
    Factory &SetStops(const std::vector<std::string> &stops);
    Factory &SetRoundTripMark(bool is_roundtrip);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    BusData data_;
  };

protected:
  void Process(QueryVisitor &visitor) const override;

private:
  BusData data_;
};

class StatResponse final : public Response {
public:
  using Response::Response;
  StatResponse(int id, const BusStat &data);
  void accept(Outputter &outputter) override;
  [[nodiscard]] int getId() const;
  [[nodiscard]] std::string getName() const;
  [[nodiscard]] uint getStopsCount() const;
  [[nodiscard]] uint getUniqueStopsCount() const;
  [[nodiscard]] double getGeoLength() const;
  [[nodiscard]] double getRouteLength() const;
  class Factory : public ResponseFactory {
  public:
    using ResponseFactory::ResponseFactory;
    Factory &SetResponse(const BusStat &data);
    [[nodiscard]] std::unique_ptr<Response> Construct(int id) const override;

  private:
    BusStat data_;
  };

private:
  int id_{};
  BusStat data_;
};

class StatBusQuery final : public ComputeQuery {
public:
  using ComputeQuery::ComputeQuery;
  StatBusQuery(int requestId, const std::string &name);

  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetName(const std::string &name);
    Factory &SetId(int requestId);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    std::string name_;
    int id_;
  };

protected:
  void Process(QueryVisitor &visitor) const override;

private:
  const int id_;
  const std::string name_;
};
} // namespace bus

namespace stop {
class AddStopQuery final : public ModifyQuery {
public:
  using ModifyQuery::ModifyQuery;
  AddStopQuery(const StopData &data);
  ~AddStopQuery() override = default;
  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetName(const std::string &name);
    Factory &SetCoordinates(double latitude, double longitude);
    Factory &SetDistances(std::map<std::string, double> road_distances);

    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    StopData data_;
  };

protected:
  void Process(QueryVisitor &visitor) const override;

private:
  StopData data_;
};

class StatResponse final : public Response {
public:
  using Response::Response;

  StatResponse(int id, const StopStat &data);
  void accept(Outputter &outputter) override;
  [[nodiscard]] int getId() const;
  [[nodiscard]] std::string getName() const;
  [[nodiscard]] std::vector<std::string>::const_iterator buses_cbegin() const;
  [[nodiscard]] std::vector<std::string>::const_iterator buses_cend() const;
  [[nodiscard]] std::vector<std::string> getBuses() const;

  class Factory : public ResponseFactory {
  public:
    using ResponseFactory::ResponseFactory;
    Factory &SetResponse(const StopStat &data);
    [[nodiscard]] std::unique_ptr<Response> Construct(int id) const override;

  private:
    StopStat data_;
  };

private:
  int id_{};
  StopStat data_;
};

class StatStopQuery final : public ComputeQuery {
public:
  using ComputeQuery::ComputeQuery;
  StatStopQuery(int requestId, const std::string &name);

  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetName(const std::string &name);
    Factory &SetId(int requestId);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    std::string name_{};
    int id_{};
  };

protected:
  void Process(QueryVisitor &visitor) const override;

private:
  const int id_;
  const std::string name_;
};
} // namespace stop

namespace map {
class RenderSettings final : public Query {
public:
  using Query::Query;
  RenderSettings(const renderer::RenderSettings &settings);
  //  ~AddStopQuery() override = default;
  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetSettings(const renderer::RenderSettings &settings);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    renderer::RenderSettings settings_;
  };
  void Execute(QueryVisitor &visitor) const override;

protected:
  void Process(QueryVisitor &visitor) const;

private:
  renderer::RenderSettings settings_;
};

class MapResponse final : public Response {
public:
  using Response::Response;
  MapResponse(int id, const std::string &data);
  void accept(Outputter &outputter) override;
  [[nodiscard]] int getId() const;
  [[nodiscard]] std::string getMap() const;

  class Factory : public ResponseFactory {
  public:
    using ResponseFactory::ResponseFactory;
    Factory &SetResponse(const std::string &data);
    [[nodiscard]] std::unique_ptr<Response> Construct(int id) const override;

  private:
    std::string data_;
  };

private:
  int id_;
  std::string data_;
};

class MapRender final : public ComputeQuery {
public:
  using ComputeQuery::ComputeQuery;
  MapRender(int requestId);

  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetId(int requestId);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    int id_;
  };

protected:
  void Process(QueryVisitor &visitor) const override;

private:
  int id_;
};
} // namespace map

namespace router {
class RoutingSettings final : public Query {
public:
  using Query::Query;
  RoutingSettings(const ::transport::router::RoutingSettings &settings);
  //  ~AddStopQuery() override = default;
  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetBusWaitTime(double time);
    Factory &SetBusVelocity(double velocity);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    ::transport::router::RoutingSettings settings_;
  };
  void Execute(QueryVisitor &visitor) const override;

protected:
  void Process(QueryVisitor &visitor) const;

private:
  ::transport::router::RoutingSettings settings_;
};

class RouteResponse final : public Response {
public:
  using Response::Response;
  RouteResponse(int id, const RouteStat &data);
  void accept(Outputter &outputter) override;
  [[nodiscard]] int getId() const;
  [[nodiscard]] double getTotalTime() const;
  [[nodiscard]] size_t items_size() const;
  [[nodiscard]] std::vector<RouteItem>::const_iterator items_cbegin() const;
  [[nodiscard]] std::vector<RouteItem>::const_iterator items_cend() const;
  //  [[nodiscard]] std::vector<std::string> getItems() const;

  class Factory : public ResponseFactory {
  public:
    using ResponseFactory::ResponseFactory;
    Factory &SetResponse(const RouteStat &data);
    [[nodiscard]] std::unique_ptr<Response> Construct(int id) const override;

  private:
    RouteStat data_;
  };

private:
  int id_;
  RouteStat data_;
};

class RouteQuery final : public ComputeQuery {
public:
  using ComputeQuery::ComputeQuery;
  RouteQuery(int id, const std::string &stop_from, const std::string &stop_to);

  class Factory : public QueryFactory {
  public:
    using QueryFactory::QueryFactory;
    Factory &SetId(int requestId);
    Factory &SetFromStop(const std::string &from);
    Factory &SetToStop(const std::string &to);
    [[nodiscard]] uniqueQuery Construct() const override;

  private:
    int id_;
    std::string stop_from_{};
    std::string stop_to_{};
  };

protected:
  void Process(QueryVisitor &visitor) const override;

private:
  int id_{};
  std::string stop_from_{};
  std::string stop_to_{};
};
} // namespace router
} // namespace queries

// Базовый абстрактный класс Inputter - интерфейс для классов, реализующих
// ввод данных в различных форматах
class Inputter {
public:
  using IOClass = std::istream;
  constexpr static const char *const type = "Inputter";

  Inputter() = default;
  Inputter(const Inputter &other) = delete;
  Inputter(Inputter &&other) = delete;
  Inputter &operator=(const Inputter &other) = delete;
  Inputter &operator=(Inputter &&other) = delete;
  virtual ~Inputter() = default;

  virtual void Parse() = 0;
  [[nodiscard]] virtual uniqueQueryList::const_iterator cbegin() const = 0;
  [[nodiscard]] virtual uniqueQueryList::iterator begin() = 0;
  [[nodiscard]] virtual uniqueQueryList::const_iterator cend() const = 0;
  [[nodiscard]] virtual uniqueQueryList::iterator end() = 0;
  static std::unique_ptr<Inputter> Construct(const std::string &id,
                                             std::istream stream);
};

class RequestHandler : public QueryVisitor {
public:
  RequestHandler(std::unique_ptr<Inputter> inputter,
                 std::unique_ptr<Outputter> outputter);
  // Получает из входного потока запросы, обрабатывает и записывает ответы в
  // указанный выходной поток
  void Execute();

  [[nodiscard]] TransportCatalogue *getCatalog() const override;
  [[nodiscard]] MapRenderer *getRenderer() const override;
  [[nodiscard]] TransportRouter *getRouter() const override;
  void appendResponse(uniqueResponse) override;

private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и
  // "Визуализатор Карты" через владение уникальным указателем
  std::unique_ptr<Inputter> inputter_;
  std::unique_ptr<Outputter> outputter_;
  std::unique_ptr<TransportCatalogue> db_;
  std::unique_ptr<MapRenderer> renderer_;
  std::unique_ptr<TransportRouter> router_;
};

} // namespace transport
