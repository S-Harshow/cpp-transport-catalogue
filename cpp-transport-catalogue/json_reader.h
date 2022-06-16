#pragma once

#include "domain.h"
#include "json.h"
#include <memory>
#include <unordered_map>

using namespace json;

namespace transport {

// Базовый абстрактный класс Outputter - интерфейс для классов, реализующих
// вывод данных в различных форматах
class Outputter {
public:
  //  virtual ~Outputter() = default;
  virtual void SetResponses(const Responses &) = 0;
};
// Базовый абстрактный класс Inputter - интерфейс для классов, реализующих
// ввод данных в различных форматах
class Inputter {
public:
  //  virtual ~Inputter() = default;
  virtual void ParseInput() = 0;
  [[nodiscard]] virtual std::optional<Requests>
      GetRequests(RequestType) const = 0;
};

// Сохранение данных формата JSON в поток
class JsonOutputter final : public Outputter {
public:
  explicit JsonOutputter(std::ostream &output_stream = std::cout);
  void SetResponses(const Responses &responses) override;

private:
  std::ostream &output_stream_;
};

// Чтение данных из потока в формате JSON,и их обработка
class JsonInputter final : public Inputter {
public:
  explicit JsonInputter(std::istream &input_stream = std::cin);
  // Чтение данных из потока и их обработка

  void ParseInput() override;
  [[nodiscard]] std::optional<Requests>
  GetRequests(RequestType request_type) const override;

private:
  std::istream &input_stream_;
  std::unordered_map<RequestType, Requests> requests_repo_{};
};
} // namespace transport

namespace transport::io::json::detail {
class Parser {
public:
  virtual std::optional<Requests> parseSection(const Node &map);
  [[nodiscard]] virtual RequestType GetType() const = 0;

protected:
  virtual std::optional<Request> parseBusNode(const Dict &map);
  virtual std::optional<Request> parseStopNode(const Dict &map);
  virtual std::optional<Request> parseMapNode(const Dict &map);

  ~Parser() = default;
};
class ParserBase final : public Parser {
public:
  [[nodiscard]] RequestType GetType() const override;
};
class ParserStat final : public Parser {
public:
  [[nodiscard]] RequestType GetType() const override;
  std::optional<Request> parseBusNode(const Dict &map) override;
  std::optional<Request> parseStopNode(const Dict &map) override;
  std::optional<Request> parseMapNode(const Dict &map) override;
};
class ParserRenderSettings final : public Parser {
public:
  std::optional<Requests> parseSection(const Node &node) override;
  [[nodiscard]] RequestType GetType() const override;
};

class ParserBuilder {
public:
  // Создать класс-разборщик
  static std::shared_ptr<Parser> CreateParser(const std::string &parser_type);
};

// Преобразование из содержимого Response в JSON
struct JsonNodeBuilder {
  // Построить ноду для StopInfo
  [[nodiscard]] Node operator()(const StopStat &info) const;
  // Построить ноду для StopInfo
  [[nodiscard]] Node operator()(const BusStat &info) const;
  // Построить ноду для MapStat
  [[nodiscard]] Node operator()(const MapStat &info) const;
  [[nodiscard]] Node operator()(const std::monostate & /*unused*/) const;
  int request_id;
};

} // namespace transport::io::json::detail
