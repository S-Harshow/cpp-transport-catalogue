#pragma once

#include "request_handler.h"
#include "json.h"
#include <memory>
#include <unordered_map>

// using namespace json;

namespace transport {

// Сохранение данных формата JSON в поток
class JsonOutputter final : public Outputter {
public:
  using Outputter::Outputter;
  explicit JsonOutputter(std::ostream &output_stream);
  static void Register();
  static std::unique_ptr<Outputter> Construct(std::ostream &stream);
  void Send() override;
  void visit(queries::EmptyResponse *response) override;
  void visit(queries::bus::StatResponse *response) override;
  void visit(queries::stop::StatResponse *response) override;
  void visit(queries::map::MapResponse *response) override;
  void visit(queries::router::RouteResponse *response) override;

private:
  std::ostream &output_stream_;
  json::Array root_array_{};
};

// Чтение данных из потока в формате JSON,и их обработка
class JsonInputter final : public Inputter {
public:
  explicit JsonInputter(std::istream &input_stream);
  static void Register();
  static std::unique_ptr<Inputter> Construct(std::istream &stream);

  // Чтение данных из потока и их обработка
  [[nodiscard]] uniqueQueryList::const_iterator cbegin() const override;
  [[nodiscard]] uniqueQueryList::iterator begin() override;
  [[nodiscard]] uniqueQueryList::const_iterator cend() const override;
  [[nodiscard]] uniqueQueryList::iterator end() override;

  void Parse() override;

private:
  std::istream &input_stream_;
  uniqueQueryList requests_{};
};

namespace io::json::detail {

class Parser {
public:
  Parser() = default;
  Parser(const Parser &other) = default;
  Parser(Parser &&other) = default;
  Parser &operator=(const Parser &other) = delete;
  Parser &operator=(Parser &&other) = delete;

  [[nodiscard]] virtual uniqueQueryList
  parseSection(const ::json::Node &map) const = 0;

  virtual ~Parser() = default;
};

class ParserBase final : public Parser {
public:
  friend class ParserStat;
  [[nodiscard]] uniqueQueryList
  parseSection(const ::json::Node &map) const override;

private:
  static uniqueQuery parseBusNode(const ::json::Dict &map);
  static uniqueQuery parseStopNode(const ::json::Dict &map);
};

class ParserStat final : public Parser {
public:
  //  [[nodiscard]] RequestType GetType() const override;
  [[nodiscard]] uniqueQueryList
  parseSection(const ::json::Node &map) const override;

private:
  static uniqueQuery parseBusNode(const ::json::Dict &map);
  static uniqueQuery parseStopNode(const ::json::Dict &map);
  static const uniqueQuery parseMapNode(const ::json::Dict &map);
  static const uniqueQuery parseRouteNode(const ::json::Dict &map);
};

class ParserRenderSettings final : public Parser {
public:
  [[nodiscard]] uniqueQueryList
  parseSection(const ::json::Node &node) const override;
  //  [[nodiscard]] RequestType GetType() const override;
};

class ParserRoutingSettings final : public Parser {
public:
  [[nodiscard]] uniqueQueryList
  parseSection(const ::json::Node &node) const override;
};

class ParserBuilder {
public:
  // Создать класс-разборщик
  static const Parser &CreateParser(std::string_view parser_type);
};

} // namespace io::json::detail
} // namespace transport
