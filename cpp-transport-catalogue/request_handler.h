#pragma once

#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include <memory>
#include <unordered_set>

namespace transport {
// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие с
// Транспортным каталогом

class RequestHandler {
public:
  // MapRenderer понадобится в следующей части итогового проекта
  explicit RequestHandler(TransportCatalogue &db, MapRenderer &renderer);

  void SetOutputter(std::shared_ptr<Outputter> outputter);

  void SetInputter(std::shared_ptr<Inputter> inputter);

  // Читает в указанном входном потоке запросы, записывает ответы в указанный
  // выходной поток
  void ProcessRequests();

  // Этот метод будет нужен в следующей части итогового проекта
  [[nodiscard]] svg::Document RenderMap() const;

private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и
  // "Визуализатор Карты"
  TransportCatalogue &db_;
  MapRenderer &renderer_;

  std::shared_ptr<Outputter> outputter_{};
  std::shared_ptr<Inputter> inputter_{};
};

} // namespace transport

namespace transport::detail {
struct BaseRequestHandler {
  // Выполнить запрос на добавление Остановки
  void operator()(const StopQuery &query) const;

  // Выполнить запрос на добавление Автобуса
  void operator()(const BusQuery &query) const;

  // Заглушки
  void operator()(const MapQuery & /*query*/) const {};
  void operator()(const renderer::RenderSettings & /*settings*/) const {};
  void operator()(const std::monostate & /*unused*/) const;

  TransportCatalogue &db_;
};
struct StatRequestHandler {
  // Выполнить запрос на получение статданных об Остановке
  [[nodiscard]] Response operator()(const StopQuery &query) const;

  // Выполнить запрос на получение статданых об Автобусе
  [[nodiscard]] Response operator()(const BusQuery &query) const;

  // Выполнить запрос на построение карты маршрутов
  [[nodiscard]] Response operator()(const MapQuery &query) const;

  [[nodiscard]] Response
  operator()(const renderer::RenderSettings & /*settings*/) const {
    return {{}, 0};
  };
  [[nodiscard]] Response operator()(const std::monostate & /*unused*/) const;

  TransportCatalogue &db_;
  const MapRenderer &renderer_;
  int request_id_;
};
} // namespace transport::detail
