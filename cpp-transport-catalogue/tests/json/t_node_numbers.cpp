#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

class LibJson_NumberNode : public ::testing::Test {
protected:
  const Node int_node{42};
  const Node dbl_node{123.45};
};

TEST_F(LibJson_NumberNode, int_node_is_int) { ASSERT_TRUE(int_node.IsInt()); };

TEST_F(LibJson_NumberNode, int_node_can_be_double) {
  // целые числа являются подмножеством чисел с плавающей запятой
  ASSERT_TRUE(int_node.IsDouble());
};

TEST_F(LibJson_NumberNode, int_node_value) {
  ASSERT_EQ(42, int_node.AsInt());
  // Когда узел хранит int, можно получить соответствующее ему double-значение
  ASSERT_DOUBLE_EQ(42.0, int_node.AsDouble());
};

TEST_F(LibJson_NumberNode, int_node_is_not_double) {
  ASSERT_FALSE(int_node.IsPureDouble());
};

TEST_F(LibJson_NumberNode, int_nodes_equal) { ASSERT_EQ(Node{42}, int_node); };

TEST_F(LibJson_NumberNode, int_node_not_equal_to_double_node) {
  // int и double - разные типы, поэтому не равны, даже когда хранят
  ASSERT_NE(Node{42.0}, int_node);
};

TEST_F(LibJson_NumberNode, double_node_is_double) {
  ASSERT_TRUE(dbl_node.IsDouble());
  ASSERT_TRUE(
      dbl_node.IsPureDouble()); // Значение содержит число с плавающей запятой
};

TEST_F(LibJson_NumberNode, double_node_value) {
  ASSERT_DOUBLE_EQ(123.45, dbl_node.AsDouble());
};

TEST_F(LibJson_NumberNode, double_node_is_not_int) {
  ASSERT_FALSE(dbl_node.IsInt());
};

TEST_F(LibJson_NumberNode, print_int_node) {
  ASSERT_EQ("42"s, Print(int_node));
  ASSERT_EQ("-42"s, Print(Node{-42}));
};

TEST_F(LibJson_NumberNode, print_double_node) {
  ASSERT_EQ("123.45"s, Print(dbl_node));
  ASSERT_EQ("-3.5"s, Print(Node{-3.5}));
};

TEST_F(LibJson_NumberNode, load_int_node) {
  ASSERT_EQ(LoadJSON("42"s).GetRoot(), int_node);
  ASSERT_EQ(LoadJSON("0").GetRoot(), Node{0});
  ASSERT_EQ(LoadJSON("-123456"s).GetRoot().AsInt(), -123456);
};

TEST_F(LibJson_NumberNode, load_double_node) {
  ASSERT_EQ(LoadJSON("123.45"s).GetRoot(), dbl_node);
  ASSERT_DOUBLE_EQ(LoadJSON("0.25"s).GetRoot().AsDouble(), 0.25);
  ASSERT_DOUBLE_EQ(LoadJSON("3e5"s).GetRoot().AsDouble(), 3e5);
  ASSERT_DOUBLE_EQ(LoadJSON("1.2e-5"s).GetRoot().AsDouble(), 1.2e-5);
  ASSERT_DOUBLE_EQ(LoadJSON("1.2e+5"s).GetRoot().AsDouble(), 1.2e5);
  ASSERT_EQ(LoadJSON("0.0").GetRoot(), Node{0.0});
};

TEST_F(LibJson_NumberNode, load_int_node_w_trash) {
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQ(LoadJSON(" \t\r\n\n\r 42 \t\r\n\n\r ").GetRoot(), int_node);
};

TEST_F(LibJson_NumberNode, load_double_node_w_trash) {
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQ(LoadJSON(" \t\r\n\n\r 0.0 \t\r\n\n\r ").GetRoot(), Node{0.0});
};
