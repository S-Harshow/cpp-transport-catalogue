#include "test_example_functions.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "test_framework.h"
#include "transport_catalogue.h"
#include <execution>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace transport::tests {
using namespace std;

using namespace transport::io;

// Функция TestSearchServer является точкой входа для запуска тестов
void TestTransportCatalogue() {
  TestRunner tester;
  tester.RunTest(TestStringToQuery, "Test string >> query");
  tester.RunTest(TestCinToQueries, "Test cin >> queries");
  //  tester.RunTest(TestCatalogFilling);
  //  tester.RunTest(TestBusTsA);
  //  tester.RunTest(TestStopTsB);
  tester.RunTest(TestCatalogFillingWithRouteDistance,
                 "Test catalog filling with route distances");
  tester.RunTest(TestStopTsC,
                 "Test catalog filling with route distances (tsC)");
}

void TestByFiles(const string &inputFileName, const string &responseFilePath) {
  ifstream input_file(inputFileName);
  Assert(input_file.is_open(), "cannot open input file " + inputFileName);

  vector<Query> queryToFillCatalog = stream::read(input_file);
  vector<Query> requests = stream::read(input_file);
  input_file.close();

  transport::TransportCatalogue catalog;
  fillCatalogue(&catalog, queryToFillCatalog);
  auto responses = executeRequests(&catalog, requests);
  //  cout << *responses << endl;
  ASSERT_EQUAL(requests.size(), responses->size());
  //проверка результата
  string response_text{};
  for_each((*responses).begin(), (*responses).end(),
           [&response_text](const Response &response) {
             response_text += response.toString() + "\r\n";
           });
  hash<string> hash_string;
  ifstream master_result(responseFilePath);
  string master_text{istreambuf_iterator<char>(master_result),
                     istreambuf_iterator<char>()};
  master_result.close();
  AssertEqual(hash_string(master_text), hash_string(response_text),
              " file names:" + responseFilePath);
}

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении
// документов
void TestStringToQuery() {
  const string add_stop = "Stop Tolstopaltsevo: 55.611087, 37.208290"s;
  const string add_bus = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s;
  const string get_bus = "Bus 750"s;
  {
    Query test = io::detail::GetQuery(add_stop);
    ASSERT_EQUAL(test.type, QueryType::STOP);
    ASSERT_EQUAL(test.name, "Tolstopaltsevo"s);
    ASSERT_EQUAL(test.data, "55.611087, 37.208290"s);
  }
  {
    Query test = io::detail::GetQuery(add_bus);
    ASSERT_EQUAL(test.type, QueryType::BUS);
    ASSERT_EQUAL(test.name, "750"s);
    ASSERT_EQUAL(test.data, "Tolstopaltsevo - Marushkino - Rasskazovka"s);
  }
  {
    Query test = io::detail::GetQuery(get_bus);
    ASSERT_EQUAL(test.type, QueryType::BUS);
    ASSERT_EQUAL(test.name, "750"s);
    ASSERT_EQUAL(test.data, ""s);
  }
}
void TestCinToQueries() {
  istringstream input;
  input.clear();
  input.str("2\n"s + "Stop Tolstopaltsevo: 55.611087, 37.208290\n"s +
            "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"s);
  vector<Query> query = stream::read(input);
  ASSERT_EQUAL(query.size(), 2);
  {
    Query request = query.at(0);
    ASSERT_EQUAL(request.type, QueryType::STOP);
    ASSERT_EQUAL(request.name, "Tolstopaltsevo"s);
    ASSERT_EQUAL(request.data, "55.611087, 37.208290"s);
  }
  {
    Query request = query.at(1);
    ASSERT_EQUAL(request.type, QueryType::BUS);
    ASSERT_EQUAL(request.name, "750"s);
    ASSERT_EQUAL(request.data, "Tolstopaltsevo - Marushkino - Rasskazovka"s);
  };
}
void TestCatalogFilling() {
  istringstream input;
  input.clear();
  const char *text = R"(10
      Stop Tolstopaltsevo: 55.611087, 37.208290
      Stop Marushkino: 55.595884, 37.209755
      Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
      Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
      Stop Rasskazovka: 55.632761, 37.333324
      Stop Biryulyovo Zapadnoye: 55.574371, 37.651700
      Stop Biryusinka: 55.581065, 37.648390
      Stop Universam: 55.587655, 37.645687
      Stop Biryulyovo Tovarnaya: 55.592028, 37.653656
      Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164
      3
      Bus 256
      Bus 750
      Bus 751)";
  input.str(text);
  vector<Query> queryToFillCatalog = stream::read(input);
  ASSERT_EQUAL(queryToFillCatalog.size(), 10);
  TransportCatalogue catalog;
  fillCatalogue(&catalog, queryToFillCatalog);

  ASSERT_EQUAL(catalog.getStopsCount() + catalog.getBusesCount(),
               queryToFillCatalog.size());
  ASSERT_EQUAL(catalog.getStopsCount(), 8);
  ASSERT_EQUAL(catalog.getBusesCount(), 2);

  vector<Query> requests = stream::read(input);
  ASSERT_EQUAL(requests.size(), 3);
  auto responses = executeRequests(&catalog, requests);
  ASSERT(responses);
  //  cout << *responses << endl;
  ASSERT_EQUAL(
      (*responses).at(0).toString(),
      "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length");
  ASSERT_EQUAL(
      (*responses).at(1).toString(),
      "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length");
  ASSERT_EQUAL((*responses).at(2).toString(), "Bus 751: not found");
};
void TestBusTsA() {
  //
  const string input_file_name_case1(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt1/tsA_case1_input.txt"s);
  const string output_file_name_case1(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt1/tsA_case1_output.txt"s);
  TestByFiles(input_file_name_case1, output_file_name_case1);

  const string input_file_name_case2(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt1/tsA_case2_input.txt"s);
  const string output_file_name_case2(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt1/tsA_case2_output.txt"s);
  TestByFiles(input_file_name_case2, output_file_name_case2);
}

void TestStopTsB() {
  const string input_file_name_case1(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt2/tsB_case1_input.txt"s);
  const string output_file_name_case1(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt2/tsB_case1_output.txt"s);
  TestByFiles(input_file_name_case1, output_file_name_case1);
  const string input_file_name_case2(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt2/tsB_case2_input.txt"s);
  const string output_file_name_case2(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt2/tsB_case2_output.txt"s);
  TestByFiles(input_file_name_case2, output_file_name_case2);
}

void TestCatalogFillingWithRouteDistance() {
  istringstream input;
  input.clear();
  const char *text = R"(13
        Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
        Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino
        Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
        Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka
        Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino
        Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
        Stop Biryusinka: 55.581065, 37.64839, 750m to Universam
        Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
        Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
        Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye
        Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
        Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
        Stop Prazhskaya: 55.611678, 37.603831
        6
        Bus 256
        Bus 750
        Bus 751
        Stop Samara
        Stop Prazhskaya
        Stop Biryulyovo Zapadnoye
  )";
  input.str(text);
  vector<Query> queryToFillCatalog = stream::read(input);
  ASSERT_EQUAL(queryToFillCatalog.size(), 13);
  TransportCatalogue catalog;
  fillCatalogue(&catalog, queryToFillCatalog);

  ASSERT_EQUAL(catalog.getStopsCount() + catalog.getBusesCount(),
               queryToFillCatalog.size());

  vector<Query> requests = stream::read(input);
  ASSERT_EQUAL(requests.size(), 6);
  auto responses = executeRequests(&catalog, requests);
  ASSERT(responses);
  //  cout << *responses << endl;

  ASSERT_EQUAL((*responses).at(0).toString(),
               "Bus 256: 6 stops on route, 5 unique stops, 5950 route length, "
               "1.36124 curvature");
  ASSERT_EQUAL((*responses).at(1).toString(),
               "Bus 750: 7 stops on route, 3 unique stops, 27400 route length, "
               "1.30853 curvature");
  ASSERT_EQUAL((*responses).at(2).toString(), "Bus 751: not found");
  ASSERT_EQUAL((*responses).at(3).toString(), "Stop Samara: not found");
  ASSERT_EQUAL((*responses).at(4).toString(), "Stop Prazhskaya: no buses");
  ASSERT_EQUAL((*responses).at(5).toString(),
               "Stop Biryulyovo Zapadnoye: buses 256 828");
};

void TestStopTsC() {
  const string input_file_name_case1(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt3/tsC_case1_input.txt"s);
  const string output_file_name_case1(
      "/home/harshow/Projects/cpp_practicum/cpp-transport-catalogue/"s
      "cpp-transport-catalogue/pt3/tsC_case1_output1.txt"s);
  TestByFiles(input_file_name_case1, output_file_name_case1);
}
// --------- Окончание модульных тестов поисковой системы -----------
} // namespace transport::tests
