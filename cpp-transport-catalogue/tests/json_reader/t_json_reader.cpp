#include "../../src/json_reader.h"
#include "../../src/request_handler.h"
#include "../../src/transport_catalogue/transport_catalogue.h"
#include "fakehandler.h"
#include "path.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <variant>

using namespace std;
using namespace transport;
using namespace filesystem;

class JsonReader : public ::testing::Test {
protected:
  void SetUp() override {
    JsonInputter::Register();
    JsonOutputter::Register();
    json_inputter = IOFactory<Inputter>::instance().Make("json", input_stream);
    auto json_outputter =
        IOFactory<Outputter>::instance().Make("json", output_stream);
    fake_catalog = make_shared<MockTransportCatalogue>();
    fake_render = make_shared<MockRenderer>();
    fake_router = make_shared<MockRouter>();
    fake_handler =
        make_unique<MockHandler>(move(json_inputter), move(json_outputter),
                                 fake_catalog, fake_render, fake_router);
  }
  std::shared_ptr<MockTransportCatalogue> fake_catalog{};
  std::shared_ptr<MockRouter> fake_router{};
  std::shared_ptr<MockRenderer> fake_render{};
  std::unique_ptr<MockHandler> fake_handler;
  std::unique_ptr<Inputter> json_inputter;
  std::istringstream input_stream{};
  std::ostringstream output_stream{};
};

TEST_F(JsonReader, Check_Render_Settings_from_json) {
  input_stream.str(
      R"({"render_settings":
  {
    "width": 1200.0,
    "height": 1200.0,

    "padding": 50.0,

    "line_width": 14.0,
    "stop_radius": 5.0,

    "bus_label_font_size": 20,
    "bus_label_offset": [7.0, 15.0],

    "stop_label_font_size": 20,
    "stop_label_offset": [7.0, -3.0],

    "underlayer_color": [255, 255, 255, 0.85],
    "underlayer_width": 3.0,

    "color_palette": [
      "green",
      [255, 160, 0],
      "red"
    ]
  }
})");
  fake_handler->Execute();
  ASSERT_EQ(1, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  const renderer::RenderSettings &settings = fake_render->settings_;
  ASSERT_TRUE(settings.isValid());
  ASSERT_DOUBLE_EQ(1200.0, settings.width());
  ASSERT_DOUBLE_EQ(1200.0, settings.height());
  ASSERT_DOUBLE_EQ(50.0, settings.padding());
  ASSERT_DOUBLE_EQ(14.0, settings.lineWidth());
  ASSERT_DOUBLE_EQ(5.0, settings.stopRadius());
  ASSERT_EQ(20, settings.busLabelFontSize());
  ASSERT_DOUBLE_EQ(7.0, settings.busLabelOffset().x);
  ASSERT_DOUBLE_EQ(15.0, settings.busLabelOffset().y);
  ASSERT_EQ(20, settings.stopLabelFontSize());
  ASSERT_DOUBLE_EQ(7.0, settings.stopLabelOffset().x);
  ASSERT_DOUBLE_EQ(-3.0, settings.stopLabelOffset().y);
  ASSERT_TRUE(svg::Color(svg::Rgba(255, 255, 255, 0.85)) ==
              settings.underlayerColor());
  ASSERT_DOUBLE_EQ(3.0, settings.underlayerWidth());
};

TEST_F(JsonReader, Check_Base_Request_bus_from_json) {
  input_stream.str(
      R"({"base_requests":[
{"type":"Bus","name":"750","stops": ["Tolstopaltsevo","Marushkino"],"is_roundtrip": false}
]})");
  fake_handler->Execute();
  ASSERT_EQ(1, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  ASSERT_EQ(1, fake_catalog->add_bus_requests.size());
  const auto &new_bus = fake_catalog->add_bus_requests.begin();
  ASSERT_EQ("750"s, new_bus->name);
  ASSERT_FALSE(new_bus->is_roundtrip);
  ASSERT_EQ(2, new_bus->stops.size());
  ASSERT_EQ("Tolstopaltsevo"s, new_bus->stops.at(0));
  ASSERT_EQ("Marushkino"s, new_bus->stops.at(1));
};

TEST_F(JsonReader, Check_Base_Request_stops_from_json) {
  input_stream.str(
      R"({"base_requests":[
    {"type": "Stop","name": "Tolstopaltsevo","latitude": 55.611087,"longitude": 37.20829,"road_distances":
{"Marushkino": 3900}},
    {"type": "Stop","name":
"Marushkino","latitude": 55.595884,"longitude": 37.209755,"road_distances":
{"Tolstopaltsevo": 9900}}
      ]})");
  fake_handler->Execute();
  ASSERT_EQ(2, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  ASSERT_EQ(2, fake_catalog->add_Stop_requests.size());

  const auto &first_stop = fake_catalog->add_Stop_requests.front();
  ASSERT_EQ("Tolstopaltsevo"s, first_stop.name);
  ASSERT_DOUBLE_EQ(55.611087, first_stop.coordinates.lat);
  ASSERT_DOUBLE_EQ(37.20829, first_stop.coordinates.lng);
  ASSERT_EQ(1, first_stop.road_distances.size());
  const auto &first_stop_dist = first_stop.road_distances.begin();
  ASSERT_EQ("Marushkino"s, first_stop_dist->first);
  ASSERT_EQ(3900, first_stop_dist->second);

  const auto &second_stop = fake_catalog->add_Stop_requests.back();
  ASSERT_EQ("Marushkino"s, second_stop.name);
  ASSERT_DOUBLE_EQ(55.595884, second_stop.coordinates.lat);
  ASSERT_DOUBLE_EQ(37.209755, second_stop.coordinates.lng);
  ASSERT_EQ(1, second_stop.road_distances.size());
  const auto &second_stop_dist = second_stop.road_distances.begin();
  ASSERT_EQ("Tolstopaltsevo"s, second_stop_dist->first);
  ASSERT_EQ(9900, second_stop_dist->second);
};

TEST_F(JsonReader, Check_Stat_Request_bus_from_json) {
  input_stream.str(
      R"({"stat_requests":[
{"id": 12345678,"type": "Bus","name": "750"}
]})");
  fake_handler->Execute();
  ASSERT_EQ(1, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  ASSERT_EQ("750"s, fake_catalog->bus_to_stat);
};

TEST_F(JsonReader, Check_Stat_Request_stop_from_json) {
  input_stream.str(
      R"({"stat_requests":[
{"id": 12345,"type": "Stop","name": "Marushkino"}
]})");
  fake_handler->Execute();
  ASSERT_EQ(1, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  ASSERT_EQ("Marushkino"s, fake_catalog->stop_to_stat);
};

TEST_F(JsonReader, Check_Map_Request_from_json) {
  input_stream.str(
      R"({"base_requests": [
      { "type": "Bus",
        "name": "114",
        "stops": ["Морской вокзал", "Ривьерский мост"],
        "is_roundtrip": false},
      { "type": "Stop",
        "name": "Ривьерский мост",
        "latitude": 43.587795,
        "longitude": 39.716901,
        "road_distances": {"Морской вокзал": 850}},
      { "type": "Stop",
        "name": "Морской вокзал",
        "latitude": 43.581969,
        "longitude": 39.719848,
        "road_distances": {"Ривьерский мост": 850}}],
    "stat_requests": [
      { "id": 1, "type": "Map" }
    ]})");
  fake_handler->Execute();
  ASSERT_EQ(4, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  ASSERT_EQ(1, fake_render->buses_for_map.size());
  ASSERT_EQ("114"s, fake_render->buses_for_map.begin()->name);
  ASSERT_FALSE(fake_render->buses_for_map.begin()->is_roundtrip);
  ASSERT_EQ(2, fake_render->buses_for_map.begin()->stops.size());
};

TEST_F(JsonReader, Check_Routing_Settings_from_json) {
  input_stream.str(
      R"({"routing_settings": {
      "bus_wait_time": 6,
      "bus_velocity": 40
}})");
  fake_handler->Execute();
  ASSERT_EQ(1, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  ASSERT_EQ(6, fake_router->settings.bus_wait);
  ASSERT_EQ(40, fake_router->settings.bus_velocity);
};

TEST_F(JsonReader, Check_Stat_Request_route_from_json) {
  input_stream.str(
      R"({"stat_requests":[
{"type": "Route","from": "Biryulyovo Zapadnoye","to": "Universam","id": 4}
]})");
  fake_handler->Execute();
  ASSERT_EQ(1, std::distance(fake_handler->Inputter_begin(),
                             fake_handler->Inputter_end()));
  //  ASSERT_EQ("Marushkino", fake_catalog->stop_to_stat);
};

TEST_F(JsonReader, Check_Router_Response_to_json) {
  RouteStat route;
  route.total_time = 11.235f;
  RouteItemStop item_stop{"Biryulyovo Zapadnoye"sv, 6};
  RouteItemBus item_bus{"297"sv, 2, 5.235f};
  route.items.push_back(item_stop);
  route.items.push_back(item_bus);
  auto response =
      queries::router::RouteResponse::Factory().SetResponse(route).Construct(4);
  fake_handler->appendResponse(move(response));
  fake_handler->SendOutput();
  ASSERT_EQ(R"([
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
    }
])",
            output_stream.str());
};
