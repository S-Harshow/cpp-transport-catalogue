#include "../../src/json_reader.h"
#include "../../src/request_handler.h"
#include "../../src/transport_catalogue/transport_catalogue.h"
#include "path.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

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
  ofstream tester_file(tester_file_name, ios::out);
  tester_file << output_stream.str();
  tester_file.close();
  ASSERT_EQ(hash_string(master_text), hash_string(output_stream.str()))
      << " tester file name:"s << tester_file_name;
}

TEST(MapRenderer, Check_Settings_from_json) {
  // FIXME use MockObject
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
  std::ostringstream output_stream;
  auto json_inputter = std::make_unique<JsonInputter>(input_stream);
  auto json_outputter = std::make_unique<JsonOutputter>(output_stream);
  RequestHandler handler{move(json_inputter), move(json_outputter)};
  handler.Execute();
  ASSERT_EQ(output_stream.str(), ""s);
};

TEST(MapRenderer, Check_JSON_from_file_pt5_w_SVG) {
  const path test_file_path =
      path(TEST_FILES_PATH) / "tests/renderer/pt5/input2.json"s;
  const path master_file_path =
      path(TEST_FILES_PATH) / "tests/renderer/pt5/output2.json"s;

  TestByFiles(test_file_path, master_file_path);
}

TEST(MapRenderer, s10_final_opentest_1) {
  const path test_file_path =
      path(TEST_FILES_PATH) / "tests/renderer/pt6/s10_final_opentest_1.json"s;
  const path master_file_path =
      path(TEST_FILES_PATH) /
      "tests/renderer/pt6/s10_final_opentest_1_answer.json"s;

  TestByFiles(test_file_path, master_file_path);
}
TEST(MapRenderer, s10_final_opentest_2) {
  const path test_file_path =
      path(TEST_FILES_PATH) / "tests/renderer/pt6/s10_final_opentest_2.json"s;

  const path master_file_path =
      path(TEST_FILES_PATH) /
      "tests/renderer/pt6/s10_final_opentest_2_answer.json"s;
  TestByFiles(test_file_path, master_file_path);
}

TEST(MapRenderer, s10_final_opentest_3) {
  const path test_file_path =
      path(TEST_FILES_PATH) / "tests/renderer/pt6/s10_final_opentest_3.json"s;

  const path master_file_path =
      path(TEST_FILES_PATH) /
      "tests/renderer/pt6/s10_final_opentest_3_answer.json"s;
  TestByFiles(test_file_path, master_file_path);
}
