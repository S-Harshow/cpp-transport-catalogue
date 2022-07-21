
#include "../../src/json_reader.h"
#include "../../src/request_handler.h"
#include "../../src/transport_catalogue/domain.h"
#include "../../src/transport_catalogue/transport_catalogue.h"
#include "../json/test_helper.h"
#include "path.h"

#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>
#include <ostream>

using namespace json;

using namespace std;
using namespace transport;
using namespace filesystem;

// Замер скорости ввода-вывода json-документа
static void LibJson_print_and_load(benchmark::State &state) {
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
  for (auto _ : state) {
    std::stringstream strm;
    json::Print(Document{Node(arr)}, strm);
    const auto doc = json::Load(strm);
    assert(doc.GetRoot() == Node(arr));
  }
}

static void TransportRouteTest_Test4(benchmark::State &state) {
  const path test_file_path =
      path(TEST_FILES_PATH) / "tests/transport_router/e4_input.json"s;
  ifstream input_file(test_file_path);
  std::ostringstream output_stream;
  auto json_inputter = std::make_unique<JsonInputter>(input_file);
  auto json_outputter = std::make_unique<JsonOutputter>(output_stream);

  RequestHandler handler{move(json_inputter), move(json_outputter)};
  //  (void)state;
  for (auto _ : state) {
    handler.Execute();
  }
}

BENCHMARK(LibJson_print_and_load);
BENCHMARK(TransportRouteTest_Test4);
