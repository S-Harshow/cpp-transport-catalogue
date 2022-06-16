#include "test_transport_catalogue.h"
#include "domain.h"
#include "json_reader.h"
#include "request_handler.h"
#include <fstream>
#include <functional>
#include <string>
//#include "input_reader.h"
//#include "stat_reader.h"

//#include <cassert>
//#include <chrono>
//#include <fstream>
//#include <sstream>
//#include <string_view>

using namespace std;
inline constexpr auto TEST_FILES_PATH =
    "/home/harshow/Projects/cpp_practicum/sprint10/transport_catalogue/sprint10_transport_catalogue/"sv;
namespace transport::test {

using namespace io;
// void TestByFiles(const string &inputFileName, const string &responseFilePath)
// {
//   ifstream input_file(inputFileName);
//   Assert(input_file.is_open(), "cannot open input file "s + inputFileName);

//  vector<Query> queryToFillCatalog = io::stream::read(input_file);
//  vector<Query> requests = io::stream::read(input_file);
//  input_file.close();

//  transport::TransportCatalogue catalog;
//  fillCatalogue(&catalog, queryToFillCatalog);
//  auto responses = executeRequests(&catalog, requests);
//  //  cout << *responses << endl;
//  ASSERT_EQUAL(requests.size(), responses->size());
//  //проверка результата
//  string response_text{};
//  for_each((*responses).begin(), (*responses).end(),
//           [&response_text](const Response &response) {
//             response_text += response.toString() + "\r\n";
//           });
//  hash<string> hash_string;
//  ifstream master_result(responseFilePath);
//  string master_text{istreambuf_iterator<char>(master_result),
//                     istreambuf_iterator<char>()};
//  master_result.close();
//  AssertEqual(hash_string(master_text), hash_string(response_text),
//              " file names:" + responseFilePath);
//}

//// -------- Начало модульных тестов поисковой системы ----------

//// Тест проверяет, что поисковая система исключает стоп-слова при добавлении
//// документов
TESTCASE(TransportCatalogue, Check_Bus_response_3stops_lap_json) {
  std::istringstream input_stream;
  input_stream.str(
      R"({"base_requests":[{"type": "Stop","name": "Tolstopaltsevo","latitude": 55.611087,"longitude": 37.20829,"road_distances": {"Marushkino": 3900}},
{"type": "Stop","name": "Marushkino","latitude": 55.595884,"longitude": 37.209755,"road_distances": {"Rasskazovka": 9900,"Marushkino": 100}},
{"type": "Stop","name": "Rasskazovka","latitude": 55.632761,"longitude": 37.333324,"road_distances": {"Marushkino": 9500}},
{"type":"Bus","name":"750","stops": ["Tolstopaltsevo","Marushkino","Marushkino","Rasskazovka"],"is_roundtrip": true}],
"stat_requests":[{"id": 12345678,"type": "Bus","name": "750"}]})");
  auto json_inputter = std::make_shared<transport::JsonInputter>(input_stream);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;

  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  ASSERT_EQUAL(
      output_stream.str(),
      R"([{"curvature":1.32764,"request_id":12345678,"route_length":13900,"stop_count":4,"unique_stop_count":3}])");
};

TESTCASE(TransportCatalogue, Check_Bus_response_2stops_nolap_json) {
  std::istringstream input_stream;
  input_stream.str(
      R"({"base_requests":[
    {"type": "Stop","name": "Tols topaltsevo","latitude": 55.611087,"longitude": 37.20829,"road_distances": {"Marushkino": 3900}},
    {"type": "Stop","name": "Marushkino","latitude": 55.595884,"longitude": 37.209755,"road_distances": {"Tols topaltsevo": 9900}},
    {"type":"Bus","name":"750","stops": ["Tols topaltsevo","Marushkino"],"is_roundtrip": false}
],
"stat_requests":[
                {"id": 12345678,"type": "Bus","name": "750"},{
  "id": 12345,
  "type": "Stop",
  "name": "Marushkino"
}
                ]
})");
  auto json_inputter = std::make_shared<transport::JsonInputter>(input_stream);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  //  std::cout << output_stream.str() << std::endl << std::endl;
  ASSERT_EQUAL(
      output_stream.str(),
      R"([{"curvature":4.07561,"request_id":12345678,"route_length":13800,"stop_count":3,"unique_stop_count":2},{"buses":["750"],"request_id":12345}])");
};
// TESTCASE(Catalogue_lib, StringToQuery) {
//   const string add_stop = "Stop Tolstopaltsevo: 55.611087, 37.208290"s;
//   const string add_bus = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
//   "s; const string get_bus = " Bus 750 "s;
//   {
//     Query test = detail::GetQuery(add_stop);
//     ASSERT_EQUAL(test.type, QueryType::STOP);
//     ASSERT_EQUAL(test.name, "Tolstopaltsevo"s);
//     ASSERT_EQUAL(test.data, "55.611087, 37.208290"s);
//   }
//   {
//     Query test = detail::GetQuery(add_bus);
//     ASSERT_EQUAL(test.type, QueryType::BUS);
//     ASSERT_EQUAL(test.name, "750"s);
//     ASSERT_EQUAL(test.data, "Tolstopaltsevo - Marushkino - Rasskazovka"s);
//   }
//   {
//     Query test = io::detail::GetQuery(get_bus);
//     ASSERT_EQUAL(test.type, QueryType::BUS);
//     ASSERT_EQUAL(test.name, "750"s);
//     ASSERT_EQUAL(test.data, ""s);
//   }
// };

// TESTCASE(Catalogue_lib, CinToQueries) {
//   istringstream input;
//   input.clear();
//   input.str("2\n"s + "Stop Tolstopaltsevo: 55.611087, 37.208290\n"s +
//             "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"s);
//   vector<Query> query = stream::read(input);
//   ASSERT_EQUAL(query.size(), 2);
//   {
//     Query request = query.at(0);
//     ASSERT_EQUAL(request.type, QueryType::STOP);
//     ASSERT_EQUAL(request.name, "Tolstopaltsevo"s);
//     ASSERT_EQUAL(request.data, "55.611087, 37.208290"s);
//   }
//   {
//     Query request = query.at(1);
//     ASSERT_EQUAL(request.type, QueryType::BUS);
//     ASSERT_EQUAL(request.name, "750"s);
//     ASSERT_EQUAL(request.data, "Tolstopaltsevo - Marushkino - Rasskazovka"s);
//   };
// };

// TESTCASE(Catalogue_lib, DISABLED_CatalogFilling) {
//   istringstream input;
//   input.clear();
//   const char *text = R"(10
//        Stop Tolstopaltsevo: 55.611087, 37.208290
//        Stop Marushkino: 55.595884, 37.209755
//        Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo
//        Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye Bus 750:
//        Tolstopaltsevo - Marushkino - Rasskazovka Stop
//        Rasskazovka: 55.632761, 37.333324 Stop Biryulyovo
//        Zapadnoye: 55.574371, 37.651700 Stop Biryusinka: 55.581065, 37.648390
//        Stop Universam: 55.587655, 37.645687
//        Stop Biryulyovo Tovarnaya: 55.592028, 37.653656
//        Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164
//        3
//        Bus 256
//        Bus 750
//        Bus 751)";
//   input.str(text);
//   vector<Query> queryToFillCatalog = stream::read(input);
//   ASSERT_EQUAL(queryToFillCatalog.size(), 10);
//   TransportCatalogue catalog;
//   fillCatalogue(&catalog, queryToFillCatalog);

//  ASSERT_EQUAL(catalog.getStopsCount() + catalog.getBusesCount(),
//               queryToFillCatalog.size());
//  ASSERT_EQUAL(catalog.getStopsCount(), 8);
//  ASSERT_EQUAL(catalog.getBusesCount(), 2);

//  vector<Query> requests = stream::read(input);
//  ASSERT_EQUAL(requests.size(), 3);
//  auto responses = executeRequests(&catalog, requests);
//  ASSERT(responses);
//  //  cout << *responses << endl;
//  ASSERT_EQUAL(
//      (*responses).at(0).toString(),
//      "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length");
//  ASSERT_EQUAL(
//      (*responses).at(1).toString(),
//      "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length");
//  ASSERT_EQUAL((*responses).at(2).toString(), "Bus 751: not found");
//};

// TESTCASE(Catalogue_lib, DISABLED_TestBusTsA) {
//   //
//   const string input_file_name_case1(string(TEST_FILES_PATH) +
//                                      "pt1/tsA_case1_input.txt"s);
//   const string output_file_name_case1(string(TEST_FILES_PATH) +
//                                       "pt1/tsA_case1_output.txt"s);
//   TestByFiles(input_file_name_case1, output_file_name_case1);

//  const string input_file_name_case2(string(TEST_FILES_PATH) +
//                                     "pt1/tsA_case2_input.txt"s);
//  const string output_file_name_case2(string(TEST_FILES_PATH) +
//                                      "pt1/tsA_case2_output.txt"s);
//  TestByFiles(input_file_name_case2, output_file_name_case2);
//}

// TESTCASE(Catalogue_lib, DISABLED_StopTsB) {
//   const string input_file_name_case1(string(TEST_FILES_PATH) +
//                                      "pt2/tsB_case1_input.txt"s);
//   const string output_file_name_case1(string(TEST_FILES_PATH) +
//                                       "pt2/tsB_case1_output.txt"s);
//   TestByFiles(input_file_name_case1, output_file_name_case1);
//   const string input_file_name_case2(string(TEST_FILES_PATH) +
//                                      "pt2/tsB_case2_input.txt"s);
//   const string output_file_name_case2(string(TEST_FILES_PATH) +
//                                       "pt2/tsB_case2_output.txt"s);
//   TestByFiles(input_file_name_case2, output_file_name_case2);
// }

// TESTCASE(Catalogue_lib, CatalogFillingWithRouteDistance) {
//   istringstream input;
//   input.clear();
//   const char *text = R"(13
//          Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
//          Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to
//          Marushkino Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam >
//          Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo
//          Zapadnoye Bus 750: Tolstopaltsevo - Marushkino - Marushkino -
//          Rasskazovka Stop Rasskazovka: 55.632761, 37.333324, 9500m to
//          Marushkino Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to
//          Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam Stop
//          Biryusinka: 55.581065, 37.64839, 750m to Universam Stop
//          Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa,
//          900m to Biryulyovo Tovarnaya Stop Biryulyovo
//          Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
//          Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to
//          Biryulyovo Zapadnoye Bus 828: Biryulyovo Zapadnoye > Universam >
//          Rossoshanskaya ulitsa > Biryulyovo Zapadnoye Stop Rossoshanskaya
//          ulitsa: 55.595579, 37.605757 Stop Prazhskaya: 55.611678, 37.603831
//          6
//          Bus 256
//          Bus 750
//          Bus 751
//          Stop Samara
//          Stop Prazhskaya
//          Stop Biryulyovo Zapadnoye
//    )";
//   input.str(text);
//   vector<Query> queryToFillCatalog = stream::read(input);
//   ASSERT_EQUAL(queryToFillCatalog.size(), 13);
//   TransportCatalogue catalog;
//   fillCatalogue(&catalog, queryToFillCatalog);

//  ASSERT_EQUAL(catalog.getStopsCount() + catalog.getBusesCount(),
//               queryToFillCatalog.size());

//  vector<Query> requests = stream::read(input);
//  ASSERT_EQUAL(requests.size(), 6);
//  auto responses = executeRequests(&catalog, requests);
//  ASSERT(responses);
//  //  cout << *responses << endl;

//  ASSERT_EQUAL((*responses).at(0).toString(),
//               "Bus 256: 6 stops on route, 5 unique stops, 5950 route length,
//               " "1.36124 curvature");
//  ASSERT_EQUAL((*responses).at(1).toString(),
//               "Bus 750: 7 stops on route, 3 unique stops, 27400 route length,
//               " "1.30853 curvature");
//  ASSERT_EQUAL((*responses).at(2).toString(), "Bus 751: not found");
//  ASSERT_EQUAL((*responses).at(3).toString(), "Stop Samara: not found");
//  ASSERT_EQUAL((*responses).at(4).toString(), "Stop Prazhskaya: no buses");
//  ASSERT_EQUAL((*responses).at(5).toString(),
//               "Stop Biryulyovo Zapadnoye: buses 256 828");
//};

// TESTCASE(Catalogue_lib, StopTsC) {
//   const string input_file_name_case1(string(TEST_FILES_PATH) +
//                                      "pt3/tsC_case1_input.txt"s);
//   const string output_file_name_case1(string(TEST_FILES_PATH) +
//                                       "pt3/tsC_case1_output1.txt"s);
//   TestByFiles(input_file_name_case1, output_file_name_case1);
// }
//// --------- Окончание модульных тестов поисковой системы -----------
} // namespace transport::test
