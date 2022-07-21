
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

void TestByFiles(const path &test_file_path, const path &master_file_path) {
  ifstream input_file(test_file_path);
  ASSERT_TRUE(input_file.is_open())
      << "cannot open input file "sv << test_file_path.generic_string() << endl;

  std::ostringstream output_stream;
  auto json_inputter = std::make_unique<JsonInputter>(input_file);
  auto json_outputter = std::make_unique<JsonOutputter>(output_stream);

  RequestHandler handler{move(json_inputter), move(json_outputter)};
  handler.Execute();

  ifstream master_file(master_file_path);
  ASSERT_TRUE(input_file.is_open())
      << "cannot open input file "sv << master_file_path.generic_string()
      << endl;
  hash<string> hash_string;
  string master_text{istreambuf_iterator<char>(master_file),
                     istreambuf_iterator<char>()};
  master_file.close();

  const string &tester_file_name =
      path(master_file_path.parent_path() / master_file_path.filename())
          .string() +
      "_tester.json"s;
  cout << test_file_path << endl;
  ofstream tester_file(tester_file_name, ios::out);
  tester_file << output_stream.str();
  tester_file.close();
  ASSERT_EQ(hash_string(master_text), hash_string(output_stream.str()))
      << " tester file name:" << tester_file_name;
}

class TransportRoute : public ::testing::Test {
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

TEST_F(TransportRoute, Check_Routing_Test1_json) {
  // clang-formatter off
  input_stream.str(
      R"(  {
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryulyovo Zapadnoye"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "635",
              "stops": [
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Prazhskaya"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.574371,
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 2600
              },
              "type": "Stop"
          },
          {
              "latitude": 55.587655,
              "longitude": 37.645687,
              "name": "Universam",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 1380,
                  "Biryulyovo Zapadnoye": 2500,
                  "Prazhskaya": 4650
              },
              "type": "Stop"
          },
          {
              "latitude": 55.592028,
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "road_distances": {
                  "Universam": 890
              },
              "type": "Stop"
          },
          {
              "latitude": 55.611717,
              "longitude": 37.603938,
              "name": "Prazhskaya",
              "road_distances": {},
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 40,
          "bus_wait_time": 6
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "297",
              "type": "Bus"
          },
          {
              "id": 2,
              "name": "635",
              "type": "Bus"
          },
          {
              "id": 3,
              "name": "Universam",
              "type": "Stop"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 4,
              "to": "Universam",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 5,
              "to": "Prazhskaya",
              "type": "Route"
          }
      ]
  }
  )");
  handler->Execute();
  ASSERT_EQ(R"([
    {
        "curvature": 1.42963,
        "request_id": 1,
        "route_length": 5990,
        "stop_count": 4,
        "unique_stop_count": 3
    },
    {
        "curvature": 1.30156,
        "request_id": 2,
        "route_length": 11570,
        "stop_count": 5,
        "unique_stop_count": 3
    },
    {
        "buses": [
            "297",
            "635"
        ],
        "request_id": 3
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 2,
                "time": 5.235,
                "type": "Bus"
            }
        ],
        "request_id": 4,
        "total_time": 11.235
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 1,
                "time": 3.9,
                "type": "Bus"
            },
            {
                "stop_name": "Biryulyovo Tovarnaya",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "635",
                "span_count": 2,
                "time": 8.31,
                "type": "Bus"
            }
        ],
        "request_id": 5,
        "total_time": 24.21
    }
])",
            output_stream.str());
  // clang-formatter on
};

TEST_F(TransportRoute, Check_Routing_Test2_json) {
  // clang-formatter off
  input_stream.str(
      R"(  {
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryusinka",
                  "Apteka",
                  "Biryulyovo Zapadnoye"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "635",
              "stops": [
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryusinka",
                  "TETs 26",
                  "Pokrovskaya",
                  "Prazhskaya"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "828",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "TETs 26",
                  "Biryusinka",
                  "Universam",
                  "Pokrovskaya",
                  "Rossoshanskaya ulitsa"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.574371,
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 2600,
                  "TETs 26": 1100
              },
              "type": "Stop"
          },
          {
              "latitude": 55.587655,
              "longitude": 37.645687,
              "name": "Universam",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 1380,
                  "Biryusinka": 760,
                  "Pokrovskaya": 2460
              },
              "type": "Stop"
          },
          {
              "latitude": 55.592028,
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "road_distances": {
                  "Universam": 890
              },
              "type": "Stop"
          },
          {
              "latitude": 55.581065,
              "longitude": 37.64839,
              "name": "Biryusinka",
              "road_distances": {
                  "Apteka": 210,
                  "TETs 26": 400
              },
              "type": "Stop"
          },
          {
              "latitude": 55.580023,
              "longitude": 37.652296,
              "name": "Apteka",
              "road_distances": {
                  "Biryulyovo Zapadnoye": 1420
              },
              "type": "Stop"
          },
          {
              "latitude": 55.580685,
              "longitude": 37.642258,
              "name": "TETs 26",
              "road_distances": {
                  "Pokrovskaya": 2850
              },
              "type": "Stop"
          },
          {
              "latitude": 55.603601,
              "longitude": 37.635517,
              "name": "Pokrovskaya",
              "road_distances": {
                  "Rossoshanskaya ulitsa": 3140
              },
              "type": "Stop"
          },
          {
              "latitude": 55.595579,
              "longitude": 37.605757,
              "name": "Rossoshanskaya ulitsa",
              "road_distances": {
                  "Pokrovskaya": 3210
              },
              "type": "Stop"
          },
          {
              "latitude": 55.611717,
              "longitude": 37.603938,
              "name": "Prazhskaya",
              "road_distances": {
                  "Pokrovskaya": 2260
              },
              "type": "Stop"
          },
          {
              "is_roundtrip": false,
              "name": "750",
              "stops": [
                  "Tolstopaltsevo",
                  "Rasskazovka"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.611087,
              "longitude": 37.20829,
              "name": "Tolstopaltsevo",
              "road_distances": {
                  "Rasskazovka": 13800
              },
              "type": "Stop"
          },
          {
              "latitude": 55.632761,
              "longitude": 37.333324,
              "name": "Rasskazovka",
              "road_distances": {},
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 30,
          "bus_wait_time": 2
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "297",
              "type": "Bus"
          },
          {
              "id": 2,
              "name": "635",
              "type": "Bus"
          },
          {
              "id": 3,
              "name": "828",
              "type": "Bus"
          },
          {
              "id": 4,
              "name": "Universam",
              "type": "Stop"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 5,
              "to": "Apteka",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 6,
              "to": "Pokrovskaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 7,
              "to": "Pokrovskaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 8,
              "to": "Biryulyovo Zapadnoye",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 9,
              "to": "Prazhskaya",
              "type": "Route"
          },
          {
              "from": "Apteka",
              "id": 10,
              "to": "Biryulyovo Tovarnaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 11,
              "to": "Tolstopaltsevo",
              "type": "Route"
          }
      ]
  })");
  handler->Execute();
  ASSERT_EQ(R"([
    {
        "curvature": 1.36159,
        "request_id": 1,
        "route_length": 5880,
        "stop_count": 6,
        "unique_stop_count": 5
    },
    {
        "curvature": 1.12195,
        "request_id": 2,
        "route_length": 14810,
        "stop_count": 11,
        "unique_stop_count": 6
    },
    {
        "curvature": 1.31245,
        "request_id": 3,
        "route_length": 15790,
        "stop_count": 11,
        "unique_stop_count": 6
    },
    {
        "buses": [
            "297",
            "635",
            "828"
        ],
        "request_id": 4
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "828",
                "span_count": 2,
                "time": 3,
                "type": "Bus"
            },
            {
                "stop_name": "Biryusinka",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 1,
                "time": 0.42,
                "type": "Bus"
            }
        ],
        "request_id": 5,
        "total_time": 7.42
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "828",
                "span_count": 4,
                "time": 9.44,
                "type": "Bus"
            }
        ],
        "request_id": 6,
        "total_time": 11.44
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Tovarnaya",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 1,
                "time": 1.78,
                "type": "Bus"
            },
            {
                "stop_name": "Universam",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "828",
                "span_count": 1,
                "time": 4.92,
                "type": "Bus"
            }
        ],
        "request_id": 7,
        "total_time": 10.7
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Tovarnaya",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 4,
                "time": 6.56,
                "type": "Bus"
            }
        ],
        "request_id": 8,
        "total_time": 8.56
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Tovarnaya",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "635",
                "span_count": 5,
                "time": 14.32,
                "type": "Bus"
            }
        ],
        "request_id": 9,
        "total_time": 16.32
    },
    {
        "items": [
            {
                "stop_name": "Apteka",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 1,
                "time": 2.84,
                "type": "Bus"
            },
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 1,
                "time": 5.2,
                "type": "Bus"
            }
        ],
        "request_id": 10,
        "total_time": 12.04
    },
    {
        "error_message": "not found",
        "request_id": 11
    }
])",
            output_stream.str());
  // clang-formatter on
};

TEST_F(TransportRoute, Check_Routing_Test3_json) {
  // clang-formatter off
  input_stream.str(R"({
    "base_requests": [
        {
            "is_roundtrip": true,
            "name": "289",
            "stops": [
                "Zagorye",
                "Lipetskaya ulitsa 46",
                "Lipetskaya ulitsa 40",
                "Lipetskaya ulitsa 40",
                "Lipetskaya ulitsa 46",
                "Moskvorechye",
                "Zagorye"
            ],
            "type": "Bus"
        },
        {
            "latitude": 55.579909,
            "longitude": 37.68372,
            "name": "Zagorye",
            "road_distances": {
                "Lipetskaya ulitsa 46": 230
            },
            "type": "Stop"
        },
        {
            "latitude": 55.581441,
            "longitude": 37.682205,
            "name": "Lipetskaya ulitsa 46",
            "road_distances": {
                "Lipetskaya ulitsa 40": 390,
                "Moskvorechye": 12400
            },
            "type": "Stop"
        },
        {
            "latitude": 55.584496,
            "longitude": 37.679133,
            "name": "Lipetskaya ulitsa 40",
            "road_distances": {
                "Lipetskaya ulitsa 40": 1090,
                "Lipetskaya ulitsa 46": 380
            },
            "type": "Stop"
        },
        {
            "latitude": 55.638433,
            "longitude": 37.638433,
            "name": "Moskvorechye",
            "road_distances": {
                "Zagorye": 10000
            },
            "type": "Stop"
        }
    ],
    "render_settings": {
        "bus_label_font_size": 20,
        "bus_label_offset": [
            7,
            15
        ],
        "color_palette": [
            "green",
            [
                255,
                160,
                0
            ],
            "red"
        ],
        "height": 200,
        "line_width": 14,
        "padding": 30,
        "stop_label_font_size": 20,
        "stop_label_offset": [
            7,
            -3
        ],
        "stop_radius": 5,
        "underlayer_color": [
            255,
            255,
            255,
            0.85
        ],
        "underlayer_width": 3,
        "width": 200
    },
    "routing_settings": {
        "bus_velocity": 30,
        "bus_wait_time": 2
    },
    "stat_requests": [
        {
            "id": 1,
            "name": "289",
            "type": "Bus"
        },
        {
            "from": "Zagorye",
            "id": 2,
            "to": "Moskvorechye",
            "type": "Route"
        },
        {
            "from": "Moskvorechye",
            "id": 3,
            "to": "Zagorye",
            "type": "Route"
        },
        {
            "from": "Lipetskaya ulitsa 40",
            "id": 4,
            "to": "Lipetskaya ulitsa 40",
            "type": "Route"
        }
    ]
})");
  handler->Execute();
  ASSERT_EQ(R"([
    {
        "curvature": 1.63414,
        "request_id": 1,
        "route_length": 24490,
        "stop_count": 7,
        "unique_stop_count": 4
    },
    {
        "items": [
            {
                "stop_name": "Zagorye",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "289",
                "span_count": 1,
                "time": 0.46,
                "type": "Bus"
            },
            {
                "stop_name": "Lipetskaya ulitsa 46",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "289",
                "span_count": 1,
                "time": 24.8,
                "type": "Bus"
            }
        ],
        "request_id": 2,
        "total_time": 29.26
    },
    {
        "items": [
            {
                "stop_name": "Moskvorechye",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "289",
                "span_count": 1,
                "time": 20,
                "type": "Bus"
            }
        ],
        "request_id": 3,
        "total_time": 22
    },
    {
        "items": [

        ],
        "request_id": 4,
        "total_time": 0
    }
])",
            output_stream.str());
  // clang-formatter on
};
TEST(TransportRouteTest, Check_Routing_Test4_json) {
  const path test_file_path =
      path(TEST_FILES_PATH) / "tests/transport_router/e4_input.json"s;
  const path master_file_path =
      path(TEST_FILES_PATH) / "tests/transport_router/e4_output_my.json"s;

  TestByFiles(test_file_path, master_file_path);
}

TEST(TransportRouteTest, s12_final_opentest_1) {
  const path test_file_path =
      path(TEST_FILES_PATH) /
      "tests/transport_router/s12_final_opentest/s12_final_opentest_1.json"s;
  const path master_file_path =
      path(TEST_FILES_PATH) /
      "tests/transport_router/s12_final_opentest/s12_final_opentest_1_answer.json"s;

  TestByFiles(test_file_path, master_file_path);
}

TEST(TransportRouteTest, s12_final_opentest_2) {
  const path test_file_path =
      path(TEST_FILES_PATH) /
      "tests/transport_router/s12_final_opentest/s12_final_opentest_2.json"s;
  const path master_file_path =
      path(TEST_FILES_PATH) /
      "tests/transport_router/s12_final_opentest/s12_final_opentest_2_answer.json"s;

  TestByFiles(test_file_path, master_file_path);
}

TEST(TransportRouteTest, s12_final_opentest_3) {
  const path test_file_path =
      path(TEST_FILES_PATH) /
      "tests/transport_router/s12_final_opentest/s12_final_opentest_3.json"s;
  const path master_file_path =
      path(TEST_FILES_PATH) /
      "tests/transport_router/s12_final_opentest/s12_final_opentest_3_answer.json"s;

  TestByFiles(test_file_path, master_file_path);
}
