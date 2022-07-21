#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

class LibJson_StringNode : public ::testing::Test {
protected:
  const Node str_node{"Hello, \"everybody\""s};
  const std::string escape_chars =
      R"("\r\n\t\"\\")"s; // При чтении строкового литерала последовательности
                          // \r,\n,\t,\\,\" преобразовываться в соответствующие
                          // символы. При выводе эти символы должны
                          // экранироваться, кроме \t.
};

// Проверка ноды со значением string
TEST_F(LibJson_StringNode, string_node_is_string) {
  ASSERT_TRUE(str_node.IsString());
};
TEST_F(LibJson_StringNode, string_node_value) {
  ASSERT_EQ("Hello, \"everybody\""s, str_node.AsString());
};

TEST_F(LibJson_StringNode, string_node_no_int) {
  ASSERT_FALSE(str_node.IsInt());
};

TEST_F(LibJson_StringNode, string_node_no_double) {
  ASSERT_FALSE(str_node.IsDouble());
};

TEST_F(LibJson_StringNode, print_string_node) {
  ASSERT_EQ("\"Hello, \\\"everybody\\\"\""s, Print(str_node));
};

TEST_F(LibJson_StringNode, load_node) {
  ASSERT_EQ(str_node, LoadJSON(Print(str_node)).GetRoot());
  ASSERT_EQ("\"\\r\\n\t\\\"\\\\\""s, Print(LoadJSON(escape_chars).GetRoot()));
};

TEST_F(LibJson_StringNode, load_node_w_trash) {
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQ(Node{"Hello"s},
            LoadJSON("\t\r\n\n\r \"Hello\" \t\r\n\n\r ").GetRoot());
};
