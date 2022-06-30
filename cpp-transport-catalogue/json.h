#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

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
  PrintContext Indented() const {
    return {out, indent_step, indent_step + indent};
  }
};

class ParsingError : public std::runtime_error {
public:
  using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int,
                                        double, std::string> {
public:
  using variant::variant;
  using Value = variant;

  bool IsInt() const;
  int AsInt() const;

  bool IsPureDouble() const;
  bool IsDouble() const;
  double AsDouble() const;

  bool IsBool() const;
  bool AsBool() const;

  bool IsNull() const;

  bool IsArray() const;
  Array &AsArray();
  const Array &AsArray() const;

  bool IsString() const;
  const std::string &AsString() const;

  bool IsDict() const;
  Dict &AsDict();
  const Dict &AsDict() const;

  const Value &GetValue() const;
  Value &GetValue();

  bool operator==(const Node &other) const;
};

// inline bool operator==(const Node &lhs, const Node &rhs) {
//   return (lhs.GetValue() == rhs.GetValue());
// }

inline bool operator!=(const Node &lhs, const Node &rhs) {
  return !(lhs == rhs);
}

class Document {
public:
  explicit Document(Node root) : root_(std::move(root)) {}

  const Node &GetRoot() const { return root_; }

private:
  Node root_;
};

inline bool operator==(const Document &lhs, const Document &rhs) {
  return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document &lhs, const Document &rhs) {
  return !(lhs == rhs);
}

Document Load(std::istream &input);

void Print(const Document &doc, std::ostream &output);

std::ostream &operator<<(std::ostream &stream, const Node &node);
} // namespace json
