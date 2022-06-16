#pragma once
/*
 * Место для вашей JSON-библиотеки
 */

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
  using runtime_error::runtime_error;
};

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
  std::ostream &out;
  int indent_step = 4;
  int indent = 0;

  void PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  // Возвращает новый контекст вывода с увеличенным смещением
  [[nodiscard]] PrintContext Indented() const {
    return {out, indent_step, indent_step + indent};
  }
};

class Node {
public:
  using Value =
      std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
  Node() = default;
  Node(std::nullptr_t);
  Node(const Array &array);
  Node(const Dict &map);
  Node(int value);
  Node(double value);
  Node(bool value);
  Node(const std::string &value);

  [[nodiscard]] bool AsBool() const;
  [[nodiscard]] int AsInt() const;
  [[nodiscard]] double
  AsDouble() const; // Возвращает значение типа double, если внутри
                    // хранится double либо int. В последнем случае
                    // возвращается приведённое в double значение.
  [[nodiscard]] const std::string &AsString() const;
  [[nodiscard]] const Array &AsArray() const;
  [[nodiscard]] const Dict &AsMap() const;
  [[nodiscard]] const Value &GetValue() const;

  bool IsNull() const;
  bool IsBool() const;
  bool IsInt() const;
  bool
  IsDouble() const; // Возвращает true, если в Node хранится int либо double.
  [[nodiscard]] bool
  IsPureDouble() const; // Возвращает true, если в Node хранится double.
  [[nodiscard]] bool IsString() const;
  [[nodiscard]] bool IsArray() const;
  [[nodiscard]] bool IsMap() const;

  bool operator==(const Node &other) const;
  bool operator!=(const Node &other) const;

private:
  Value value_;
};

// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value &value, json::PrintContext ctx) {
  auto &out = ctx.out;
  out << value;
}
// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, PrintContext ctx);
// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string &value, PrintContext ctx);

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(bool value, PrintContext ctx);
// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array &values, PrintContext ctx);
// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict &value, PrintContext ctx);

void PrintNode(const Node &node, json::PrintContext ctx);

std::ostream &operator<<(std::ostream &os, const Node &node);

class Document {
public:
  explicit Document(Node root);
  [[nodiscard]] const Node &GetRoot() const;
  bool operator==(const Document &other) const;
  bool operator!=(const Document &other) const;

private:
  Node root_;
};

Document Load(std::istream &input);

void Print(const Document &doc, std::ostream &output);

} // namespace json
