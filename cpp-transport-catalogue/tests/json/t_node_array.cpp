// Проверка ноды со значением vector

#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

class LibJson_ArrayNode : public ::testing::Test {
protected:
  const Array master_arr{Node(1), Node(1.23), Node("Hello"s)};
  const Node arr_node{master_arr};
};

TEST_F(LibJson_ArrayNode, array_node_is_array) {
  ASSERT_TRUE(arr_node.IsArray());
};
TEST_F(LibJson_ArrayNode, array_node_size) {
  const Array &arr = arr_node.AsArray();
  ASSERT_EQ(3, arr.size());
};

TEST_F(LibJson_ArrayNode, array_node_value_int) {
  const Array &arr = arr_node.AsArray();
  ASSERT_EQ(1, arr.at(0).AsInt());
};

TEST_F(LibJson_ArrayNode, array_node_value_double) {
  const Array &arr = arr_node.AsArray();
  ASSERT_DOUBLE_EQ(1.23, arr.at(1).AsDouble());
};

TEST_F(LibJson_ArrayNode, array_node_value_string) {
  const Array &arr = arr_node.AsArray();
  ASSERT_EQ("Hello"s, arr.at(2).AsString());
};

TEST_F(LibJson_ArrayNode, print_node) {
  ASSERT_EQ(R"([
    1,
    1.23,
    "Hello"
])"s,
            Print(arr_node));
};

TEST_F(LibJson_ArrayNode, load_node) {
  ASSERT_NO_THROW(LoadJSON("[1,1.23,\"Hello\"]"s));
  ASSERT_EQ(arr_node, LoadJSON("[1,1.23,\"Hello\"]"s).GetRoot());
  ASSERT_EQ(arr_node, LoadJSON(Print(arr_node)).GetRoot());
};

TEST_F(LibJson_ArrayNode, load_node_w_spaces) {
  ASSERT_NO_THROW(LoadJSON(R"(  [ 1  ,  1.23,  "Hello"   ]   )"s));
  ASSERT_EQ(arr_node, LoadJSON("  [ 1  ,  1.23,  \"Hello\"   ]   "s).GetRoot());
};
TEST_F(LibJson_ArrayNode, load_node_w_trash) {

  //    Пробелы, табуляции и символы перевода строки между токенами JSON файла
  //    игнорируются
  ASSERT_NO_THROW(LoadJSON(
      "[ 1 \r \n ,   \r\n\t 1.23, \n \n  \t\t  \"Hello\" \t \n  ] \n"s));
  ASSERT_EQ(
      arr_node,
      LoadJSON("[ 1 \r \n ,  \r\n\t 1.23, \n \n  \t\t  \"Hello\" \t \n  ] \n"s)
          .GetRoot());
};
