// Проверка ноды со значением vector

#include "../../src/json/json_builder.h"
#include "test_helper.h"
#include <iostream>

using namespace testing;
using namespace std::string_literals;
using namespace json;

TEST(LibJson, build_string) {
  std::ostringstream ostream;
  json::Print(json::Document{json::Builder{}.Value("just a string"s).Build()},
              ostream);

  ASSERT_EQ(ostream.str(), R"("just a string")");
};

TEST(LibJson, build_document) {
  std::ostringstream ostream;
  json::Print(json::Document{json::Builder{}
                                 .StartDict()
                                 .Key("key1"s)
                                 .Value(123)
                                 .Key("key2"s)
                                 .Value("value2"s)
                                 .Key("key3"s)
                                 .StartArray()
                                 .Value(456)
                                 .StartDict()
                                 .EndDict()
                                 .StartDict()
                                 .Key(""s)
                                 .Value(nullptr)
                                 .EndDict()
                                 .Value(""s)
                                 .EndArray()
                                 .EndDict()
                                 .Build()},
              ostream);

  ASSERT_EQ(ostream.str(), R"({
    "key1": 123,
    "key2": "value2",
    "key3": [
        456,
        {

        },
        {
            "": null
        },
        ""
    ]
})");
};
