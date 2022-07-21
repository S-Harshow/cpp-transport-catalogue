// Проверка ноды со значением map

#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

class LibJson_MapNode : public ::testing::Test {
protected:
  const Dict master_dict{{"key1"s, Node("value1"s)}, {"key2"s, Node(42)}};
  const Node dict_node{master_dict};
};

TEST_F(LibJson_MapNode, map_node_is_map) { ASSERT_TRUE(dict_node.IsDict()); };

TEST_F(LibJson_MapNode, map_node_value) {
  const Dict &dict = dict_node.AsDict();
  ASSERT_EQ(2UL, dict.size());
  ASSERT_EQ("value1"s, dict.at("key1"s).AsString());
  ASSERT_EQ(42, dict.at("key2"s).AsInt());
};

TEST_F(LibJson_MapNode, print_map_node) {
  ASSERT_EQ(R"({
    "key1": "value1",
    "key2": 42
})",
            Print(dict_node));
};

TEST_F(LibJson_MapNode, load_map_node) {
  ASSERT_EQ(dict_node,
            LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s).GetRoot());
  ASSERT_EQ(dict_node, LoadJSON(Print(dict_node)).GetRoot());
};

TEST_F(LibJson_MapNode, load_map_node_w_trash) {
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  // clang-format off
  ASSERT_EQ(
      dict_node,
      LoadJSON(
          "\t\r\n\n\r { \t\r\n\n\r \"key1\" \t\r\n\n\r: \t\r\n\n\r \"value1\"\t\r\n\n\r , \t\r\n\n\r \"key2\" \t\r\n\n\r : \t\r\n\n\r 42 \t\r\n\n\r }\t\r\n\n\r"s)
          .GetRoot());
  // clang-format on

  ASSERT_EQ(LoadJSON(R"({
        "key": "value", "key2"    : 4.31e8,
        "[]"
        :
            [[], null   ,   15.5
                 ]
         })"s)
                .GetRoot(),
            LoadJSON(R"({"[]":[[],null,15.5],"key":"value","key2":4.31e+08})"s)
                .GetRoot());
};
