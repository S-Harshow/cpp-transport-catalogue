#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

class LibJson_NullNode : public ::testing::Test {
protected:
  const Node null_node{nullptr};
  Node defualt_null_node;
};

TEST_F(LibJson_NullNode, defualt_null_node_is_null) {
  ASSERT_TRUE(defualt_null_node.IsNull());
};

TEST_F(LibJson_NullNode, null_node_is_null) { ASSERT_TRUE(null_node.IsNull()); };

TEST_F(LibJson_NullNode, null_nodes_equality) {
  ASSERT_TRUE(null_node == defualt_null_node);
  ASSERT_TRUE(!(null_node != defualt_null_node));
};

TEST_F(LibJson_NullNode, null_node_is_not_int) {
  ASSERT_FALSE(null_node.IsInt());
};

TEST_F(LibJson_NullNode, null_node_is_not_double) {
  ASSERT_FALSE(null_node.IsDouble());
};

TEST_F(LibJson_NullNode, null_node_is_not_pure_double) {
  ASSERT_FALSE(null_node.IsPureDouble());
};

TEST_F(LibJson_NullNode, null_node_is_not_string) {
  ASSERT_FALSE(null_node.IsString());
};

TEST_F(LibJson_NullNode, null_node_is_not_array) {
  ASSERT_FALSE(null_node.IsArray());
};

TEST_F(LibJson_NullNode, null_node_is_not_dict) {
  ASSERT_FALSE(null_node.IsDict());
};

TEST_F(LibJson_NullNode, print_node) { ASSERT_EQ("null"s, Print(null_node)); };

TEST_F(LibJson_NullNode, load_node) {
  const Node node = LoadJSON("null"s).GetRoot();
  ASSERT_TRUE(node.IsNull());
  ASSERT_TRUE(node == null_node);
};

TEST_F(LibJson_NullNode, load_null_node_w_trash) {
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQ(LoadJSON(" \t\r\n\n\r null \t\r\n\n\r "s).GetRoot(), null_node);
};
