
#include "../../src/json_reader.h"
#include "../../src/request_handler.h"
#include "../../src/transport_catalogue/domain.h"
#include "../../src/transport_catalogue/transport_catalogue.h"
#include "path.h"
#include <filesystem>
#include <fstream>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <functional>

using namespace std;
using namespace transport;
using namespace filesystem;

class main_Handler : public ::testing::Test {
protected:
  void SetUp() override {
    JsonOutputter::Register();
    JsonInputter::Register();
    auto json_inputter =
        IOFactory<Inputter>::instance().Make("json", input_stream);
    auto json_outputter =
        IOFactory<Outputter>::instance().Make("json", output_stream);

    handler = std::make_unique<RequestHandler>(move(json_inputter),
                                               move(json_outputter));
  }

  std::istringstream input_stream{};
  std::ostringstream output_stream{};

  std::unique_ptr<RequestHandler> handler{};
};

//// -------- Начало модульных тестов поисковой системы ----------

//// Тест проверяет, что поисковая система исключает стоп-слова при добавлении
//// документов

TEST_F(main_Handler, Check_Bus_response_3stops_lap_json) {
  // clang-formatter off
  input_stream.str(
      R"({"base_requests":[{"type": "Stop","name":
"Tolstopaltsevo","latitude": 55.611087,"longitude": 37.20829,"road_distances":
{"Marushkino": 3900}},
{"type": "Stop","name":
"Marushkino","latitude": 55.595884,"longitude": 37.209755,"road_distances":
{"Rasskazovka": 9900,"Marushkino": 100}},
{"type": "Stop","name":
"Rasskazovka","latitude": 55.632761,"longitude": 37.333324,"road_distances":
{"Marushkino": 9500}},
{"type":"Bus","name":"750","stops":
["Tolstopaltsevo","Marushkino","Marushkino","Rasskazovka"],"is_roundtrip":
true}], "stat_requests":[{"id": 12345678,"type": "Bus","name": "750"}]}
)");
  handler->Execute();
  ASSERT_EQ(R"([
    {
        "curvature": 1.32764,
        "request_id": 12345678,
        "route_length": 13900,
        "stop_count": 4,
        "unique_stop_count": 3
    }
])",
            output_stream.str());
  // clang-formatter on
};

TEST_F(main_Handler, Check_Bus_response_2stops_nolap_json) {
  // clang-formatter off
  input_stream.str(
      R"({"base_requests":[
    {"type": "Stop","name": "Tolstopaltsevo","latitude": 55.611087,"longitude": 37.20829,"road_distances":
{"Marushkino": 3900}},
    {"type": "Stop","name":
"Marushkino","latitude": 55.595884,"longitude": 37.209755,"road_distances":
{"Tolstopaltsevo": 9900}},
    {"type":"Bus","name":"750","stops": ["Tolstopaltsevo","Marushkino"],"is_roundtrip": false}
],
"stat_requests":[
                {"id": 12345678,"type": "Bus","name": "750"},{
  "id": 12345,
  "type": "Stop",
  "name": "Marushkino"
}
                ]
})");
  handler->Execute();
  ASSERT_EQ(R"([
    {
        "curvature": 4.07561,
        "request_id": 12345678,
        "route_length": 13800,
        "stop_count": 3,
        "unique_stop_count": 2
    },
    {
        "buses": [
            "750"
        ],
        "request_id": 12345
    }
])",
            output_stream.str());
  // clang-formatter on
};

//// --------- Окончание модульных тестов поисковой системы -----------
