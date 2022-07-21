// Проверка ноды со значением bool

#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

class LibJson_BoolNode : public ::testing::Test {
protected:
  const Node true_node{true};
  const Node false_node{false};
};

TEST_F(LibJson_BoolNode, bool_nodes_are_bool) {
  ASSERT_TRUE(true_node.IsBool());
  ASSERT_TRUE(false_node.IsBool());
};

TEST_F(LibJson_BoolNode, bool_nodes_value) {
  ASSERT_TRUE(true_node.AsBool());
  ASSERT_FALSE(false_node.AsBool());
};

TEST_F(LibJson_BoolNode, print_nodes) {
  ASSERT_EQ("true"s, Print(true_node));
  ASSERT_EQ("false"s, Print(false_node));
};

TEST_F(LibJson_BoolNode, load_nodes) {
  ASSERT_EQ(true_node, LoadJSON("true"s).GetRoot());
  ASSERT_EQ(false_node, LoadJSON("false"s).GetRoot());
};

TEST_F(LibJson_BoolNode, load_nodes_w_trash) {
  ASSERT_EQ(true_node, LoadJSON(" \t\r\n\n\r true \r\n"s).GetRoot());
  ASSERT_EQ(false_node, LoadJSON(" \t\r\n\n\r false \t\r\n\n\r "s).GetRoot());
};
