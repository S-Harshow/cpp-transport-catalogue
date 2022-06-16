

#include "request_handler.h"
#include "test_framework.h"
#include "transport_catalogue.h"
#include <fstream>
#include <functional>
using namespace std;

namespace transport::test {
inline constexpr auto TEST_FILES_PATH =
    "/home/harshow/Projects/cpp_practicum/sprint10/transport_catalogue/sprint10_transport_catalogue/"sv;

TESTCASE(MapRenderer, Check_Settings_from_json) {
  std::istringstream input_stream;
  input_stream.str(
      R"({"base_requests":  [],
            "stat_requests":  [],
            "render_settings":
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
  auto json_inputter = std::make_shared<transport::JsonInputter>(input_stream);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);
  ASSERT(!map_render.hasSettings());
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  ASSERT(map_render.hasSettings());
  ASSERT_EQUAL(output_stream.str(), "");
};

TESTCASE(MapRenderer, Check_SVG_from_file_pt5) {
  const string input_file_name(string(TEST_FILES_PATH) + "pt5/input.json"s);
  ifstream input_file(input_file_name);
  Assert(input_file.is_open(), "cannot open input file "s + input_file_name);

  auto json_inputter = std::make_shared<transport::JsonInputter>(input_file);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);

  ASSERT(!map_render.hasSettings());
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  auto doc = handler.RenderMap();
  ASSERT(map_render.hasSettings());
  ASSERT_EQUAL(
      output_stream.str(),
      R"([{"buses":["14"],"request_id":2},{"curvature":1.23199,"request_id":3,"route_length":1700,"stop_count":3,"unique_stop_count":2}])");
  ostringstream svg_map;
  doc.Render(svg_map);
  ASSERT_EQUAL(svg_map.str(), R"==(<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <polyline points="99.2283,329.5 50,232.18 99.2283,329.5" fill="none" stroke="green" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
  <polyline points="550,190.051 279.22,50 333.61,269.08 550,190.051" fill="none" stroke="rgb(255,160,0)" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="99.2283" y="329.5" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="green" x="99.2283" y="329.5" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="50" y="232.18" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="green" x="50" y="232.18" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="550" y="190.051" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">14</text>
  <text fill="rgb(255,160,0)" x="550" y="190.051" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">14</text>
  <circle cx="279.22" cy="50" r="5" fill="white"/>
  <circle cx="99.2283" cy="329.5" r="5" fill="white"/>
  <circle cx="50" cy="232.18" r="5" fill="white"/>
  <circle cx="333.61" cy="269.08" r="5" fill="white"/>
  <circle cx="550" cy="190.051" r="5" fill="white"/>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="279.22" y="50" dx="7" dy="-3" font-size="20" font-family="Verdana">Elektroseti</text>
  <text fill="black" x="279.22" y="50" dx="7" dy="-3" font-size="20" font-family="Verdana">Elektroseti</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="99.2283" y="329.5" dx="7" dy="-3" font-size="20" font-family="Verdana">Morskoy vokzal</text>
  <text fill="black" x="99.2283" y="329.5" dx="7" dy="-3" font-size="20" font-family="Verdana">Morskoy vokzal</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="50" y="232.18" dx="7" dy="-3" font-size="20" font-family="Verdana">Rivierskiy most</text>
  <text fill="black" x="50" y="232.18" dx="7" dy="-3" font-size="20" font-family="Verdana">Rivierskiy most</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="333.61" y="269.08" dx="7" dy="-3" font-size="20" font-family="Verdana">Ulitsa Dokuchaeva</text>
  <text fill="black" x="333.61" y="269.08" dx="7" dy="-3" font-size="20" font-family="Verdana">Ulitsa Dokuchaeva</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="550" y="190.051" dx="7" dy="-3" font-size="20" font-family="Verdana">Ulitsa Lizy Chaikinoi</text>
  <text fill="black" x="550" y="190.051" dx="7" dy="-3" font-size="20" font-family="Verdana">Ulitsa Lizy Chaikinoi</text>
</svg>
)==");
}
BENCHMARK(MapRenderer, Check_SVG_from_file_pt6, 20, 1) {
  const string input_file_name(string(TEST_FILES_PATH) +
                               "pt6/s10_final_opentest_3.json"s);
  ifstream input_file(input_file_name);
  Assert(input_file.is_open(), "cannot open input file "s + input_file_name);

  auto json_inputter = std::make_shared<transport::JsonInputter>(input_file);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);

  ASSERT(!map_render.hasSettings());
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
}

TESTCASE(MapRenderer, Check_SVG_from_file_pt5_2) {
  const string input_file_name(string(TEST_FILES_PATH) + "pt5/input2.json"s);
  ifstream input_file(input_file_name);
  Assert(input_file.is_open(), "cannot open input file "s + input_file_name);

  auto json_inputter = std::make_shared<transport::JsonInputter>(input_file);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);

  ASSERT(!map_render.hasSettings());
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  auto doc = handler.RenderMap();
  ASSERT(map_render.hasSettings());
  //  ASSERT_EQUAL(output_stream.str(), "");
  //  std::cout << output_stream.str();
  ostringstream svg_map;
  doc.Render(svg_map);
  ASSERT_EQUAL(svg_map.str(), R"==(<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <polyline points="100.817,170 30,30 100.817,170" fill="none" stroke="green" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="100.817" y="170" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="green" x="100.817" y="170" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="30" y="30" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <text fill="green" x="30" y="30" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">114</text>
  <circle cx="100.817" cy="170" r="5" fill="white"/>
  <circle cx="30" cy="30" r="5" fill="white"/>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="100.817" y="170" dx="7" dy="-3" font-size="20" font-family="Verdana">Морской вокзал</text>
  <text fill="black" x="100.817" y="170" dx="7" dy="-3" font-size="20" font-family="Verdana">Морской вокзал</text>
  <text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="30" y="30" dx="7" dy="-3" font-size="20" font-family="Verdana">Ривьерский мост</text>
  <text fill="black" x="30" y="30" dx="7" dy="-3" font-size="20" font-family="Verdana">Ривьерский мост</text>
</svg>
)==");
}
TESTCASE(MapRenderer, Check_JSON_from_file_pt5_w_SVG) {
  const string input_file_name(string(TEST_FILES_PATH) + "pt5/input2.json"s);
  ifstream input_file(input_file_name);
  Assert(input_file.is_open(), "cannot open input file "s + input_file_name);

  auto json_inputter = std::make_shared<transport::JsonInputter>(input_file);
  std::ostringstream output_stream;
  auto json_outputter =
      std::make_shared<transport::JsonOutputter>(output_stream);

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);

  ASSERT(!map_render.hasSettings());
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  ASSERT_EQUAL(
      output_stream.str(),
      R"==([{"map":"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>\n","request_id":1},{"buses":["114"],"request_id":2},{"curvature":1.23199,"request_id":3,"route_length":1700,"stop_count":3,"unique_stop_count":2}])==");
}

} // namespace transport::test
