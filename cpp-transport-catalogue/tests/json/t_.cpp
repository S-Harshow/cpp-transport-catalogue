
#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

// Проверка ноды на неверные входные данные
TEST(LibJson, Check_ErrorHandling) {
  ASSERT_ANY_THROW(LoadJSON("["s));
  ASSERT_ANY_THROW(LoadJSON("]"s));

  ASSERT_ANY_THROW(LoadJSON("{"s));
  ASSERT_ANY_THROW(LoadJSON("}"s));

  ASSERT_ANY_THROW(LoadJSON("\"hello"s)); // незакрытая кавычка

  ASSERT_ANY_THROW(LoadJSON("tru"s));
  ASSERT_ANY_THROW(LoadJSON("fals"s));
  ASSERT_ANY_THROW(LoadJSON("nul"s));

  //  std::logic_error
  Node dbl_node{3.5};
  MustThrowLogicError([&dbl_node] {
    auto var = dbl_node.AsInt();
    (void)var;
  });
  MustThrowLogicError([&dbl_node] {
    auto var = dbl_node.AsString();
    (void)var;
  });
  MustThrowLogicError([&dbl_node] {
    auto var = dbl_node.AsArray();
    (void)var;
  });

  Node array_node{Array{}};
  MustThrowLogicError([&array_node] {
    auto var = array_node.AsDict();
    (void)var;
  });
  MustThrowLogicError([&array_node] {
    auto var = array_node.AsDouble();
    (void)var;
  });
  MustThrowLogicError([&array_node] {
    auto var = array_node.AsBool();
    (void)var;
  });
};
