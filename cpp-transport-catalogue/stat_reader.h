#pragma once
#include "input_reader.h"
// чтение данных из каталога запросами
namespace transport::io {
// типы ответов
enum class ResponseType { STOP, BUS };
std::ostream &operator<<(std::ostream &, ResponseType);

// формализованная структура ответа
struct Response {
  Response(ResponseType, const std::string &, const std::string &);
  // выполняет преобразование формализованного ответа в строку
  [[nodiscard]] std::string toString() const;
  ResponseType type{};
  std::string name{};
  std::string data{};
};
std::ostream &operator<<(std::ostream &, const Response &);
std::ostream &operator<<(std::ostream &, const std::vector<Response> &);

// возвращает из справочника результат запроса
[[nodiscard]] std::optional<std::vector<Response>>
executeRequests(TransportCatalogue *, const std::vector<Query> &);

} // namespace transport::io

namespace transport::io::detail { // непосредственная обработка запросов в
                                  // справочник
std::optional<Response> getStopInfo(TransportCatalogue *, const Query &);
std::optional<Response> getBusInfo(TransportCatalogue *, const Query &);
} // namespace transport::io::detail
