/*
 * Место для вашей JSON-библиотеки
 */
#include "json.h"
#include <algorithm>
#include <iomanip>
#include <istream>
#include <iterator>
#include <limits>
#include <optional>

using namespace std;

namespace json::detail {
inline constexpr const char JSON_OBJECT_BEGIN = '{';
inline constexpr const char JSON_OBJECT_END = '}';
inline constexpr const char JSON_ARRAY_BEGIN = '[';
inline constexpr const char JSON_ARRAY_END = ']';
inline constexpr const char JSON_STRING_BEGIN = '"';
inline constexpr const char JSON_STRING_END = '"';
inline constexpr const char JSON_NAME_SEPARATOR = ':';
inline constexpr const char JSON_VALUE_SEPARATOR = ',';
inline constexpr const char *JSON_NULL = "null";
inline constexpr const char *JSON_TRUE = "true";
inline constexpr const char *JSON_FALSE = "false";
} // namespace json::detail
namespace json {

namespace {
using Number = std::variant<int, double>;

Number LoadNumber_impl(std::istream &input) {
  using namespace std::literals;

  std::string parsed_num;

  // Считывает в parsed_num очередной символ из input
  auto read_char = [&parsed_num, &input] {
    parsed_num += static_cast<char>(input.get());
    if (!input) {
      throw ParsingError("Failed to read number from stream"s);
    }
  };

  // Считывает одну или более цифр в parsed_num из input
  auto read_digits = [&input, read_char] {
    if (0 == std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (0 != std::isdigit(input.peek())) {
      read_char();
    }
  };

  if ('-' == input.peek()) {
    read_char();
  }
  // Парсим целую часть числа
  if ('0' == input.peek()) {
    read_char();
    // После 0 в JSON не могут идти другие цифры
  } else {
    read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if ('.' == input.peek()) {
    read_char();
    read_digits();
    is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch_ = input.peek(); 'e' == ch_ || 'E' == ch_) {
    read_char();
    if (ch_ = input.peek(); '+' == ch_ || '-' == ch_) {
      read_char();
    }
    read_digits();
    is_int = false;
  }

  try {
    if (is_int) {
      // Сначала пробуем преобразовать строку в int
      try {
        return std::stoi(parsed_num);
      } catch (...) {
        // В случае неудачи, например, при переполнении,
        // код ниже попробует преобразовать строку в double
      }
    }
    return std::stod(parsed_num);
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString_impl(std::istream &input) {
  using namespace std::literals;

  auto iter = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string str;
  while (true) {
    if (iter == end) {
      // Поток закончился до того, как встретили закрывающую кавычку?
      throw ParsingError("String parsing error"s);
    }
    const char ch_ = *iter;
    if ('"' == ch_) {
      // Встретили закрывающую кавычку
      ++iter;
      break;
    }
    if ('\\' == ch_) {
      // Встретили начало escape-последовательности
      ++iter;
      if (iter == end) {
        // Поток завершился сразу после символа обратной косой черты
        throw ParsingError("String parsing error"s);
      }
      const char escaped_char = *(iter);
      // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
      switch (escaped_char) {
      case 'n':
        str.push_back('\n');
        break;
      case 't':
        str.push_back('\t');
        break;
      case 'r':
        str.push_back('\r');
        break;
      case '"':
        str.push_back('"');
        break;
      case '\\':
        str.push_back('\\');
        break;
      default:
        // Встретили неизвестную escape-последовательность
        throw ParsingError("Unrecognized escape sequence \\"s +
                           to_string(escaped_char));
      }
    } else if ('\n' == ch_ || '\r' == ch_) {
      // Строковый литерал внутри- JSON не может прерываться символами \r или \n
      throw ParsingError("Unexpected end of line"s);
    } else {
      // Просто считываем очередной символ и помещаем его в результирующую
      // строку
      str.push_back(ch_);
    }
    ++iter;
  }
  return str;
}

Node LoadNode(istream &input);

Node LoadArray(istream &input) {
  Array result{};
  for (char ch_ = 0; input >> ch_;) {
    if (detail::JSON_ARRAY_END == ch_) {
      return Node(result);
    }
    if (detail::JSON_VALUE_SEPARATOR != ch_) {
      input.putback(ch_); // возвращаю первый символ значения
    }
    result.push_back(LoadNode(input));
  }
  throw ParsingError("The expected 'detail::JSON_ARRAY_END' dismiss"s);
}

Node LoadNumber(istream &input) {
  Number number = LoadNumber_impl(input);
  if (holds_alternative<int>(number)) {
    return Node(get<int>(number));
  }
  if (holds_alternative<double>(number)) {
    return Node(get<double>(number));
  }
  throw ParsingError("Number not found"s);
}

Node LoadBool(istream &input) {
  using namespace std::literals;
  bool value = false;
  if (!(input >> boolalpha >> value)) {
    input.clear();
    input.ignore();
    input >> noboolalpha;
    throw ParsingError("Bool not found"s);
  }
  return Node(value);
}

Node LoadNull(istream &input) {
  char str[5];
  input.get(str, 5);
  if (detail::JSON_NULL != string(str)) {
    throw ParsingError("detail::JSON_NULL not found");
  }
  return Node();
}

Node LoadString(istream &input) { return Node(LoadString_impl(input)); }

Node LoadDict(istream &input) {
  Dict result;
  //  while (input.peek() != istream::traits_type::eof()) {
  for (char ch_ = 0; input >> ch_;) {
    //      char ch_;
    //    input >> ch_;
    if (ch_ == detail::JSON_OBJECT_END) {
      return Node(result);
    }
    if (ch_ != detail::JSON_VALUE_SEPARATOR) {
      input.putback(ch_); // возвращаю первый символ значения
    }
    input >> ch_;
    string key = LoadString_impl(input);
    input >> ch_;
    result.insert({move(key), LoadNode(input)});
  }
  throw ParsingError("The expected 'detail::JSON_OBJECT_END' dismiss"s);
}

Node LoadNode(istream &input) {
  char ch_ = 0;
  input >> ws >> ch_;
  switch (ch_) {
  case detail::JSON_ARRAY_BEGIN:
    return LoadArray(input);
    break;
  case detail::JSON_OBJECT_BEGIN:
    return LoadDict(input);
    break;
  case detail::JSON_STRING_BEGIN:
    return LoadString(input);
    break;
  case 't':
    input.putback(ch_);
    return LoadBool(input);
    break;
  case 'f':
    input.putback(ch_);
    return LoadBool(input);
    break;
  case 'n':
    input.putback(ch_);
    return LoadNull(input);
    break;
  default:
    if ((0 != std::isdigit(ch_)) || '-' == ch_) {
      input.putback(ch_);
      return LoadNumber(input);
    }
    break;
  }
  //  }
  //  return {};
  throw ParsingError("Unkown symbol \""s + to_string(ch_) + "\""s);
}
} // namespace
//--------------- Node ---------------------

Node::Node(std::nullptr_t) : value_(nullptr) {}

Node::Node(const Array &array) : value_(array) {}

Node::Node(const Dict &map) : value_(map) {}

Node::Node(int value) : value_(value) {}

Node::Node(double value) : value_(value) {}

Node::Node(bool value) : value_(value) {}

Node::Node(const string &value) : value_(value) {}

const Node::Value &Node::GetValue() const { return value_; }

int Node::AsInt() const {
  const auto *as_int = get_if<int>(&value_);
  if (nullptr == as_int) {
    throw std::logic_error("The value is not int"s);
  }
  return *as_int;
}

const Array &Node::AsArray() const {
  const auto *as_array = get_if<Array>(&value_);
  if (nullptr == as_array) {
    throw std::logic_error("The value is not Array"s);
  }
  return *as_array;
}

const Dict &Node::AsMap() const {
  const auto *as_dict = get_if<Dict>(&value_);
  if (nullptr == as_dict) {
    throw std::logic_error("The value is not Dict"s);
  }
  return *as_dict;
}

const string &Node::AsString() const {
  const auto *as_string = get_if<string>(&value_);
  if (nullptr == as_string) {
    throw std::logic_error("The value is not string"s);
  }
  return *as_string;
}

bool Node::AsBool() const {
  const auto *as_bool = get_if<bool>(&value_);
  if (nullptr == as_bool) {
    throw std::logic_error("The value is not bool"s);
  }
  return *as_bool;
}

double Node::AsDouble() const {
  const auto *as_double = get_if<double>(&value_);
  if (nullptr == as_double) {
    const auto *as_int = get_if<int>(&value_);
    if (nullptr == as_int) {
      throw std::logic_error("The value is not double or int"s);
    }
    return static_cast<double>(*as_int);
  }
  return *as_double;
}

bool Node::IsInt() const { return holds_alternative<int>(value_); }

bool Node::IsDouble() const {
  return holds_alternative<double>(value_) || holds_alternative<int>(value_);
  ;
}

bool Node::IsPureDouble() const { return holds_alternative<double>(value_); }

bool Node::IsBool() const { return holds_alternative<bool>(value_); }

bool Node::IsString() const { return holds_alternative<string>(value_); }

bool Node::IsNull() const { return holds_alternative<nullptr_t>(value_); }

bool Node::IsArray() const { return holds_alternative<Array>(value_); }

bool Node::IsMap() const { return holds_alternative<Dict>(value_); }

bool Node::operator==(const Node &other) const {
  return (this->GetValue().index() == other.GetValue().index() &&
          this->GetValue() == other.GetValue());
}
bool Node::operator!=(const Node &other) const { return !(*this == other); }

//--------------- Document ---------------------
Document::Document(Node root) : root_(move(root)) {}

bool Document::operator==(const Document &other) const {
  return this->GetRoot() == other.GetRoot();
}
bool Document::operator!=(const Document &other) const {
  return !(*this == other);
}
const Node &Document::GetRoot() const { return root_; }

Document Load(istream &input) { return Document{LoadNode(input)}; }

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, json::PrintContext ctx) {
  auto &out = ctx.out;
  out << detail::JSON_NULL;
}
// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const string &value, json::PrintContext ctx) {
  auto &out = ctx.out;
  out << detail::JSON_STRING_BEGIN;
  for (const auto &ch_ : value) {
    switch (ch_) {
    case detail::JSON_STRING_BEGIN:
      out << "\\\""sv;
      break;
    case '\\':
      out << "\\\\"sv;
      break;
    case '\n':
      out << "\\n"sv;
      break;
    case '\r':
      out << "\\r"sv;
      break;
      //    case '\t':
      //      out << "\\t"sv;
      //      break;
    default:
      out << ch_;
      break;
    }
  }

  out << detail::JSON_STRING_END;
}

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(bool value, json::PrintContext ctx) {
  auto &out = ctx.out;
  out << boolalpha << value << noboolalpha;
}
// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array &values, json::PrintContext ctx) {
  auto &out = ctx.out;
  out << detail::JSON_ARRAY_BEGIN;
  if (!values.empty()) {
    for (auto iter = values.begin(); iter != prev(values.end()); ++iter) {
      PrintNode(*iter, ctx);
      out << detail::JSON_VALUE_SEPARATOR;
    }
    PrintNode(*prev(values.end()), ctx);
  }
  out << detail::JSON_ARRAY_END;
}
// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict &values, json::PrintContext ctx) {
  auto &out = ctx.out;
  out << detail::JSON_OBJECT_BEGIN;
  if (!values.empty()) {
    for (auto iter = values.begin(); iter != values.end(); ++iter) {
      if (iter != values.begin()) {
        out << detail::JSON_VALUE_SEPARATOR;
      }
      PrintValue(iter->first, ctx);
      out << detail::JSON_NAME_SEPARATOR;
      PrintNode(iter->second, ctx);
    }
  }
  out << detail::JSON_OBJECT_END;
}

void PrintNode(const Node &node, json::PrintContext ctx) {
  std::visit([&ctx](const auto &value) { PrintValue(value, ctx); },
             node.GetValue());
}

void Print(const Document &doc, std::ostream &output) {
  json::PrintContext ctx{output, 2, 2};
  PrintNode(doc.GetRoot(), ctx);
}

ostream &operator<<(std::ostream &stream, const Node &node) {
  json::PrintContext ctx{stream, 2, 2};
  PrintNode(node, ctx);
  return stream;
}

} // namespace json
