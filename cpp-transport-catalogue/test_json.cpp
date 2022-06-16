#include "test_json.h"
#include "json.h"
#include "json_reader.h"

#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>

using namespace std::literals;

namespace json::test {

// загрузка json-документа из строки
json::Document LoadJSON(const std::string &str) {
  std::istringstream strm(str);
  return json::Load(strm);
}

// "печать" json-документа в строку
std::string Print(const Node &node) {
  std::ostringstream out;
  Print(Document{node}, out);
  return out.str();
}

void MustFailToLoad(const std::string &str) {
  try {
    LoadJSON(str);
    std::cerr << "ParsingError exception is expected on '"sv << str << "'"sv
              << std::endl;
    assert(false);
  } catch (const json::ParsingError &) {
    // ok
  } catch (const std::exception &e) {
    std::cerr << "exception thrown: "sv << e.what() << std::endl;
    assert(false);
  } catch (...) {
    std::cerr << "Unexpected error"sv << std::endl;
    assert(false);
  }
}

//Ниже даны тесты, проверяющие JSON-библиотеку.
// Проверка ноды со значением null
TESTCASE(JSON_lib, DISABLED_Check_Null_node) {
  Node null_node;
  ASSERT(null_node.IsNull());
  ASSERT(!null_node.IsInt());
  ASSERT(!null_node.IsDouble());
  ASSERT(!null_node.IsPureDouble());
  ASSERT(!null_node.IsString());
  ASSERT(!null_node.IsArray());
  ASSERT(!null_node.IsMap());

  Node null_node1{nullptr};
  ASSERT(null_node1.IsNull());

  ASSERT_EQUAL(Print(null_node), "null"s);
  ASSERT_EQUAL(null_node, null_node1);
  ASSERT(!(null_node != null_node1));

  const Node node = LoadJSON("null"s).GetRoot();
  ASSERT(node.IsNull());
  // ASSERT_EQUAL
  ASSERT(node == null_node);
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQUAL(LoadJSON(" \t\r\n\n\r null \t\r\n\n\r "s).GetRoot(), null_node);
};

// Проверка ноды со значением int или double
TESTCASE(JSON_lib, DISABLED_Check_Numbers_node) {
  const Node int_node{42};
  ASSERT(int_node.IsInt());
  ASSERT_EQUAL(int_node.AsInt(), 42);
  // целые числа являются подмножеством чисел с плавающей запятой
  ASSERT(int_node.IsDouble());
  // Когда узел хранит int, можно получить соответствующее ему double-значение
  ASSERT_EQUAL(int_node.AsDouble(), 42.0);
  ASSERT(!int_node.IsPureDouble());
  ASSERT(int_node == Node{42});
  // int и double - разные типы, поэтому не равны, даже когда хранят
  ASSERT(int_node != Node{42.0});

  const Node dbl_node{123.45};
  ASSERT(dbl_node.IsDouble());
  ASSERT_EQUAL(dbl_node.AsDouble(), 123.45);
  ASSERT(
      dbl_node.IsPureDouble()); // Значение содержит число с плавающей запятой
  ASSERT(!dbl_node.IsInt());

  ASSERT_EQUAL(Print(int_node), "42"s);
  ASSERT_EQUAL(Print(dbl_node), "123.45"s);
  ASSERT_EQUAL(Print(Node{-42}), "-42"s);
  ASSERT_EQUAL(Print(Node{-3.5}), "-3.5"s);

  ASSERT_EQUAL(LoadJSON("42"s).GetRoot(), int_node);
  ASSERT_EQUAL(LoadJSON("123.45"s).GetRoot(), dbl_node);
  ASSERT_EQUAL(LoadJSON("0.25"s).GetRoot().AsDouble(), 0.25);
  ASSERT_EQUAL(LoadJSON("3e5"s).GetRoot().AsDouble(), 3e5);
  ASSERT_EQUAL(LoadJSON("1.2e-5"s).GetRoot().AsDouble(), 1.2e-5);
  ASSERT_EQUAL(LoadJSON("1.2e+5"s).GetRoot().AsDouble(), 1.2e5);
  ASSERT_EQUAL(LoadJSON("-123456"s).GetRoot().AsInt(), -123456);
  ASSERT_EQUAL(LoadJSON("0").GetRoot(), Node{0});
  ASSERT_EQUAL(LoadJSON("0.0").GetRoot(), Node{0.0});
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQUAL(LoadJSON(" \t\r\n\n\r 0.0 \t\r\n\n\r ").GetRoot(), Node{0.0});
};

// Проверка ноды со значением string
TESTCASE(JSON_lib, Check_Strings_node) {
  Node str_node{"Hello, \"everybody\""s};
  ASSERT(str_node.IsString());
  ASSERT_EQUAL(str_node.AsString(), "Hello, \"everybody\""s);

  ASSERT(!str_node.IsInt());
  ASSERT(!str_node.IsDouble());

  ASSERT_EQUAL(Print(str_node), "\"Hello, \\\"everybody\\\"\""s);
  ASSERT_EQUAL(LoadJSON(Print(str_node)).GetRoot(), str_node);
  const std::string escape_chars =
      R"("\r\n\t\"\\")"s; // При чтении строкового литерала последовательности
                          // \r,\n,\t,\\,\"
  // преобразовываться в соответствующие символы.
  // При выводе эти символы должны экранироваться, кроме \t.
  ASSERT_EQUAL(Print(LoadJSON(escape_chars).GetRoot()),
               "\"\\r\\n\t\\\"\\\\\""s);
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQUAL(LoadJSON("\t\r\n\n\r \"Hello\" \t\r\n\n\r ").GetRoot(),
               Node{"Hello"s});
};

// Проверка ноды со значением bool
TESTCASE(JSON_lib, Check_Bool_node) {
  Node true_node{true};
  ASSERT(true_node.IsBool());
  ASSERT(true_node.AsBool());

  Node false_node{false};
  ASSERT(false_node.IsBool());
  ASSERT(!false_node.AsBool());

  ASSERT_EQUAL(Print(true_node), "true"s);
  ASSERT_EQUAL(Print(false_node), "false"s);

  ASSERT_EQUAL(LoadJSON("true"s).GetRoot(), true_node);
  ASSERT_EQUAL(LoadJSON("false"s).GetRoot(), false_node);
  ASSERT_EQUAL(LoadJSON(" \t\r\n\n\r true \r\n"s).GetRoot(), true_node);
  ASSERT_EQUAL(LoadJSON(" \t\r\n\n\r false \t\r\n\n\r "s).GetRoot(),
               false_node);
};

// Проверка ноды со значением vector
TESTCASE(JSON_lib, Check_Array_node) {
  Node arr_node{Node({Node(1), Node(1.23), Node("Hello"s)})};
  ASSERT(arr_node.IsArray());
  const Array &arr = arr_node.AsArray();
  ASSERT_EQUAL(arr.size(), 3);
  ASSERT_EQUAL(arr.at(0).AsInt(), 1);

  ASSERT_EQUAL(LoadJSON("[1,1.23,\"Hello\"]"s).GetRoot(), arr_node);
  ASSERT_EQUAL(LoadJSON(Print(arr_node)).GetRoot(), arr_node);
  ASSERT_EQUAL(LoadJSON(R"(  [ 1  ,  1.23,  "Hello"   ]   )"s).GetRoot(),
               arr_node);
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  ASSERT_EQUAL(
      LoadJSON(
          "[ 1 \r \n ,  \r\n\t 1.23, \n \n  \t\t  \"Hello\" \t \n  ] \n  "s)
          .GetRoot(),
      arr_node);
};

// Проверка ноды со значением map
TESTCASE(JSON_lib, Check_Map_node) {
  Node dict_node(Node({{"key1"s, Node("value1"s)}, {"key2"s, Node(42)}}));
  ASSERT(dict_node.IsMap());
  const Dict &dict = dict_node.AsMap();
  ASSERT_EQUAL(dict.size(), 2);
  ASSERT_EQUAL(dict.at("key1"s).AsString(), "value1"s);
  ASSERT_EQUAL(dict.at("key2"s).AsInt(), 42);

  ASSERT_EQUAL(LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s).GetRoot(),
               dict_node);
  ASSERT_EQUAL(LoadJSON(Print(dict_node)).GetRoot(), dict_node);
  // Пробелы, табуляции и символы перевода строки между токенами JSON файла
  // игнорируются
  // clang-format off
    ASSERT_EQUAL(
        LoadJSON(
            "\t\r\n\n\r { \t\r\n\n\r \"key1\" \t\r\n\n\r: \t\r\n\n\r \"value1\"\t\r\n\n\r , \t\r\n\n\r \"key2\" \t\r\n\n\r : \t\r\n\n\r 42 \t\r\n\n\r }\t\r\n\n\r"s).GetRoot(), dict_node);

    ASSERT_EQUAL(
        LoadJSON(R"({
        "key": "value", "key2"    : 4.31e8,
        "[]"
        :
            [[], null   ,   15.5
                 ]
         })"s).GetRoot(), LoadJSON(R"({"[]":[[],null,15.5],"key":"value","key2":4.31e+08})"s).GetRoot());
  // clang-format on
};

// Проверка ноды на неверные входные данные
TESTCASE(JSON_lib, Check_ErrorHandling) {
  MustFailToLoad("["s);
  MustFailToLoad("]"s);

  MustFailToLoad("{"s);
  MustFailToLoad("}"s);

  MustFailToLoad("\"hello"s); // незакрытая кавычка

  MustFailToLoad("tru"s);
  MustFailToLoad("fals"s);
  MustFailToLoad("nul"s);

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
    auto var = array_node.AsMap();
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

// Замер скорости ввода-вывода json-документа
BENCHMARK(JSON_lib, Benchmark, 10, 1) {
  Array arr;
  arr.reserve(1'000);
  for (int i = 0; i < 1'000; ++i) {
    arr.emplace_back(Dict{
        {"int"s, 42},
        {"double"s, 42.1},
        {"null"s, nullptr},
        {"string"s, "hello"s},
        {"array"s, Array{1, 2, 3}},
        {"bool"s, true},
        {"map"s, Dict{{"key"s, "value"s}}},
    });
  }
  std::stringstream strm;
  json::Print(Document{Node(arr)}, strm);
  const auto doc = json::Load(strm);
  assert(doc.GetRoot() == Node(arr));
};

//Ниже даны тесты, проверяющие json_reader.
namespace {
using namespace ::test::detail;

// Проверка вывода результата поиска Остановки
TESTCASE(JsonOutputter, Check_StopStat_output) {
  std::ostringstream stream;
  transport::JsonOutputter json_outputter{stream};
  transport::StopStat stop_info("Гагарина"s, {"34"s, "52"s});
  transport::Response response{{stop_info}, 1};
  json_outputter.SetResponses({response});
  ASSERT_EQUAL(stream.str(), R"([{"buses":["34","52"],"request_id":1}])")
};

// Проверка вывода результата поиска Автобуса
TESTCASE(JsonOutputter, Check_BusStat_output) {
  std::ostringstream stream;
  transport::JsonOutputter json_outputter{stream};
  transport::BusStat bus_info("Гагарина"s);
  bus_info.stops = 4;
  bus_info.unique_stops = 3;
  bus_info.geolength = 4254.;
  bus_info.routelength = 9300;
  transport::Response response{{bus_info}, 12345678};
  json_outputter.SetResponses({response});
  ASSERT_EQUAL(
      stream.str(),
      R"([{"curvature":2.18618,"request_id":12345678,"route_length":9300,"stop_count":4,"unique_stop_count":3}])")
};

// Проверка вывода пустого результата поиска
TESTCASE(JsonOutputter, Check_NullInfo_output) {
  std::ostringstream stream;
  transport::JsonOutputter json_outputter{stream};

  transport::Response response{std::monostate(), 12345678};
  json_outputter.SetResponses({response});
  ASSERT_EQUAL(stream.str(),
               R"([{"error_message":"not found","request_id":12345678}])")
};

// Проверка ввода запроса для добавления в каталог Остановки
TESTCASE(JsonInputter, Check_BaseRequests_Stop_input) {
  std::istringstream stream;
  stream.str(
      R"({"base_requests":[{"type": "Stop","name":"Электросети","latitude":43.598701,"longitude":39.730623,"road_distances":{"Улица Докучаева":3000,"Улица Лизы Чайкиной":4300}
}],"stat_requests":[]})");
  transport::JsonInputter json_inputter{stream};
  json_inputter.ParseInput();
  auto base_requests = json_inputter.GetRequests(transport::RequestType::Base);
  ASSERT(base_requests.has_value())
  auto stat_requests = json_inputter.GetRequests(transport::RequestType::Stat);
  ASSERT(base_requests.has_value())
  ASSERT_EQUAL(base_requests->size(), 1);
  ASSERT_EQUAL(stat_requests->size(), 0);
  auto request = base_requests->at(0);
  ASSERT_EQUAL(request.requestId, 0);
  transport::StopQuery stop_request =
      std::get<transport::StopQuery>(request.value);
  ASSERT_EQUAL(stop_request.name, "Электросети"s);
  ASSERT_EQUAL(stop_request.latitude, 43.598701);
  ASSERT_EQUAL(stop_request.longitude, 39.730623);
  ASSERT_EQUAL(stop_request.road_distances.size(), 2);
};

// Проверка ввода запроса для добавления в каталог Автобуса
TESTCASE(JsonInputter, Check_BaseRequests_Bus_input) {
  std::istringstream stream;
  stream.str(
      R"({"base_requests":[{"type": "Bus",
      "name": "14",
      "stops": [
        "Улица Лизы Чайкиной",
        "Электросети",
        "Улица Докучаева",
        "Улица Лизы Чайкиной"
      ],
        "is_roundtrip": true
      } ],"stat_requests":[]})");
  transport::JsonInputter json_inputter{stream};
  json_inputter.ParseInput();
  auto base_requests = json_inputter.GetRequests(transport::RequestType::Base);
  ASSERT(base_requests.has_value())
  auto stat_requests = json_inputter.GetRequests(transport::RequestType::Stat);
  ASSERT(base_requests.has_value())
  ASSERT_EQUAL(base_requests->size(), 1);
  ASSERT_EQUAL(stat_requests->size(), 0);
  auto request = base_requests->at(0);
  ASSERT_EQUAL(request.requestId, 0);
  transport::BusQuery bus_request =
      std::get<transport::BusQuery>(request.value);
  ASSERT_EQUAL(bus_request.name, "14"s);
  ASSERT_EQUAL(bus_request.is_roundtrip, true);
  ASSERT_EQUAL(bus_request.stops.size(), 4);
};

// Проверка ввода запроса для поиск в каталоге Остановки
TESTCASE(JsonInputter, Check_StatRequests_Stop_input) {
  std::istringstream stream;
  stream.str(
      R"({"base_requests":[],"stat_requests":[{
                                                      "id": 12345678,
                                                      "type": "Bus",
                                                      "name": "14"
                                                    } ]})");
  transport::JsonInputter json_inputter{stream};
  json_inputter.ParseInput();
  auto base_requests = json_inputter.GetRequests(transport::RequestType::Base);
  ASSERT(base_requests.has_value())
  auto stat_requests = json_inputter.GetRequests(transport::RequestType::Stat);
  ASSERT(base_requests.has_value())
  ASSERT_EQUAL(base_requests->size(), 0);
  ASSERT_EQUAL(stat_requests->size(), 1);
  auto request = stat_requests->at(0);
  ASSERT_EQUAL(request.requestId, 12345678);
  transport::BusQuery bus_request =
      std::get<transport::BusQuery>(request.value);
  ASSERT_EQUAL(bus_request.name, "14"s);
};
} // namespace
} // namespace json::test
