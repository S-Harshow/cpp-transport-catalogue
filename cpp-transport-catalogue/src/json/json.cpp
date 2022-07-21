#include "json.h"

#include <iterator>

using namespace json;

namespace {
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

using namespace std::literals;

Node LoadNode(std::istream &input);
Node LoadString(std::istream &input);

std::string LoadLiteral(std::istream &input) {
  std::string str;
  while (std::isalpha(input.peek()) != 0) {
    str.push_back(static_cast<char>(input.get()));
  }
  return str;
}

Node LoadArray(std::istream &input) {
  std::vector<Node> result;

  for (char chr = 0; input >> chr && chr != JSON_ARRAY_END;) {
    if (chr != JSON_VALUE_SEPARATOR) {
      input.putback(chr);
    }
    result.push_back(LoadNode(input));
  }
  if (!input) {
    throw ParsingError("Array parsing error"s);
  }
  return Node(std::move(result));
}

Node LoadDict(std::istream &input) {
  Dict dict;

  for (char chr = 0; input >> chr && chr != JSON_OBJECT_END;) {
    if (chr == JSON_STRING_BEGIN) {
      std::string key = LoadString(input).AsString();
      if (input >> chr && chr == JSON_NAME_SEPARATOR) {
        if (dict.find(key) != dict.end()) {
          throw ParsingError("Duplicate key '"s + key + "' have been found");
        }
        dict.emplace(std::move(key), LoadNode(input));
      } else {
        throw ParsingError(": is expected but '"s + std::to_string(chr) +
                           "' has been found"s);
      }
    } else if (chr != JSON_VALUE_SEPARATOR) {
      throw ParsingError(R"(',' is expected but ')"s + std::to_string(chr) +
                         "' has been found"s);
    }
  }
  if (!input) {
    throw ParsingError("Dictionary parsing error"s);
  }
  return Node(std::move(dict));
}

Node LoadString(std::istream &input) {
  auto iter = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string str;
  while (true) {
    if (iter == end) {
      throw ParsingError("String parsing error");
    }
    const char chr = *iter;
    if (chr == JSON_STRING_BEGIN) {
      ++iter;
      break;
    }
    if (chr == '\\') {
      ++iter;
      if (iter == end) {
        throw ParsingError("String parsing error");
      }
      const char escaped_char = *(iter);
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
      case JSON_STRING_BEGIN:
        str.push_back(JSON_STRING_BEGIN);
        break;
      case '\\':
        str.push_back('\\');
        break;
      default:
        throw ParsingError("Unrecognized escape sequence \\"s +
                           std::to_string(escaped_char));
      }
    } else if (chr == '\n' || chr == '\r') {
      throw ParsingError("Unexpected end of line"s);
    } else {
      str.push_back(chr);
    }
    ++iter;
  }

  return Node(std::move(str));
}

Node LoadBool(std::istream &input) {
  const auto str = LoadLiteral(input);
  if (str == JSON_TRUE) {
    return Node{true};
  }
  if (str == JSON_FALSE) {
    return Node{false};
  }
  throw ParsingError("Failed to parse '"s + str + "' as bool"s);
}

Node LoadNull(std::istream &input) {
  auto literal = LoadLiteral(input);
  if (literal == JSON_NULL) {
    return Node{nullptr};
  }
  throw ParsingError("Failed to parse '"s + literal + "' as null"s);
}

Node LoadNumber(std::istream &input) {
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
    if (!std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (std::isdigit(input.peek())) {
      read_char();
    }
  };

  if (input.peek() == '-') {
    read_char();
  }
  // Парсим целую часть числа
  if (input.peek() == '0') {
    read_char();
    // После 0 в JSON не могут идти другие цифры
  } else {
    read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if (input.peek() == '.') {
    read_char();
    read_digits();
    is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
    read_char();
    if (ch = input.peek(); ch == '+' || ch == '-') {
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
        // В случае неудачи, например, при переполнении
        // код ниже попробует преобразовать строку в double
      }
    }
    return std::stod(parsed_num);
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

Node LoadNode(std::istream &input) {
  char chr = 0;
  if (!(input >> chr)) {
    throw ParsingError("Unexpected EOF"s);
  }
  switch (chr) {
  case JSON_ARRAY_BEGIN:
    return LoadArray(input);
  case JSON_OBJECT_BEGIN:
    return LoadDict(input);
  case JSON_STRING_BEGIN:
    return LoadString(input);
  case 't':
    // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
    // подсказкой компилятору и человеку, что здесь программист явно задумывал
    // разрешить переход к инструкции следующей ветки case, а не случайно забыл
    // написать break, return или throw.
    // В данном случае, встретив t или f, переходим к попытке парсинга
    // литералов true либо false
    [[fallthrough]];
  case 'f':
    input.putback(chr);
    return LoadBool(input);
  case 'n':
    input.putback(chr);
    return LoadNull(input);
  default:
    if ((0 != std::isdigit(chr)) || '-' == chr) {
      input.putback(chr);
      return LoadNumber(input);
    }
  }
  throw ParsingError("Unkown symbol \""s + chr + "(" + std::to_string(chr) +
                     ")\""s);
}

struct PrintContext {
  std::ostream &out;
  int indent_step = 4;
  int indent = 0;

  void PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  [[nodiscard]] PrintContext Indented() const {
    return {out, indent_step, indent_step + indent};
  }
};

void PrintNode(const Node &node, const PrintContext &ctx);

template <typename Value>
void PrintValue(const Value &value, PrintContext ctx) {
  ctx.out << value;
}

void PrintString(const std::string &value, std::ostream &out) {
  out.put(JSON_STRING_BEGIN);
  for (const char chr : value) {
    switch (chr) {
    case '\r':
      out << "\\r"sv;
      break;
    case '\n':
      out << "\\n"sv;
      break;
    case JSON_STRING_BEGIN:
      // Символы " и \ выводятся как \" или \\, соответственно
      [[fallthrough]];
    case '\\':
      out.put('\\');
      [[fallthrough]];
    default:
      out.put(chr);
      break;
    }
  }
  out.put(JSON_STRING_END);
}

template <>
void PrintValue<std::string>(const std::string &value, PrintContext ctx) {
  PrintString(value, ctx.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t & /*unused*/,
                                PrintContext ctx) {
  ctx.out << JSON_NULL;
}

// В специализаци шаблона PrintValue для типа bool параметр value передаётся
// по константной ссылке, как и в основном шаблоне.
// В качестве альтернативы можно использовать перегрузку:
// void PrintValue(bool value, const PrintContext& ctx);
template <> void PrintValue<bool>(const bool &value, PrintContext ctx) {
  ctx.out << (value ? JSON_TRUE : JSON_FALSE);
}

template <> void PrintValue<Array>(const Array &nodes, PrintContext ctx) {
  std::ostream &out = ctx.out;
  out << JSON_ARRAY_BEGIN << "\n"sv;
  bool first = true;
  auto inner_ctx = ctx.Indented();
  for (const Node &node : nodes) {
    if (first) {
      first = false;
    } else {
      out << JSON_VALUE_SEPARATOR << "\n"sv;
    }
    inner_ctx.PrintIndent();
    PrintNode(node, inner_ctx);
  }
  out.put('\n');
  ctx.PrintIndent();
  out.put(JSON_ARRAY_END);
}

template <> void PrintValue<Dict>(const Dict &nodes, PrintContext ctx) {
  std::ostream &out = ctx.out;
  out << JSON_OBJECT_BEGIN << "\n"sv;
  bool first = true;
  auto inner_ctx = ctx.Indented();
  for (const auto &[key, node] : nodes) {
    if (first) {
      first = false;
    } else {
      out << JSON_VALUE_SEPARATOR << "\n"sv;
    }
    inner_ctx.PrintIndent();
    PrintString(key, ctx.out);
    out << JSON_NAME_SEPARATOR << " "sv;
    PrintNode(node, inner_ctx);
  }
  out.put('\n');
  ctx.PrintIndent();
  out.put(JSON_OBJECT_END);
}

void PrintNode(const Node &node, const PrintContext &ctx) {
  std::visit([&ctx](const auto &value) { PrintValue(value, ctx); },
             node.GetValue());
}

} // namespace

json::Document json::Load(std::istream &input) {
  return Document{LoadNode(input)};
}

void json::Print(const Document &doc, std::ostream &output) {
  PrintNode(doc.GetRoot(), {output});
}

std::ostream &json::operator<<(std::ostream &stream, const Node &node) {
  PrintNode(node, {stream, 2, 2});
  return stream;
}

bool Node::IsInt() const { return std::holds_alternative<int>(*this); }

int Node::AsInt() const {
  using namespace std::literals;
  if (!IsInt()) {
    throw std::logic_error("Not an int"s);
  }
  return std::get<int>(*this);
}

bool Node::IsUint() const { return std::holds_alternative<uint>(*this); }

uint Node::AsUint() const {
  using namespace std::literals;
  if (!IsUint()) {
    throw std::logic_error("Not an uint"s);
  }
  return std::get<uint>(*this);
}

bool Node::IsPureDouble() const {
  return std::holds_alternative<double>(*this);
}

bool Node::IsDouble() const { return IsInt() || IsPureDouble(); }

double Node::AsDouble() const {
  using namespace std::literals;
  if (!IsDouble()) {
    throw std::logic_error("Not a double"s);
  }
  return IsPureDouble() ? std::get<double>(*this) : AsInt();
}

bool Node::IsBool() const { return std::holds_alternative<bool>(*this); }

bool Node::AsBool() const {
  using namespace std::literals;
  if (!IsBool()) {
    throw std::logic_error("Not a bool"s);
  }

  return std::get<bool>(*this);
}

bool Node::IsNull() const {
  return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const { return std::holds_alternative<Array>(*this); }

Array &Node::AsArray() {
  return const_cast<Array &>(static_cast<const Node &>(*this).AsArray());
}

const Array &Node::AsArray() const {
  using namespace std::literals;
  if (!IsArray()) {
    throw std::logic_error("Not an array"s);
  }

  return std::get<Array>(*this);
}

bool Node::IsString() const {
  return std::holds_alternative<std::string>(*this);
}

const std::string &Node::AsString() const {
  using namespace std::literals;
  if (!IsString()) {
    throw std::logic_error("Not a string"s);
  }

  return std::get<std::string>(*this);
}

bool Node::IsDict() const { return std::holds_alternative<Dict>(*this); }

Dict &Node::AsDict() {
  return const_cast<Dict &>(static_cast<const Node &>(*this).AsDict());
}

const Dict &Node::AsDict() const {
  using namespace std::literals;
  if (!IsDict()) {
    throw std::logic_error("Not a dict"s);
  }
  return std::get<Dict>(*this);
}

bool Node::operator==(const Node &other) const {
  return (this->GetValue().index() == other.GetValue().index() &&
          this->GetValue().valueless_by_exception() ==
              other.GetValue().valueless_by_exception());
}

const Node::Value &Node::GetValue() const { return *this; }

Node::Value &Node::GetValue() { return *this; }

//} // namespace json
