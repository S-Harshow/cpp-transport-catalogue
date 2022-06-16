
//#include "input_reader.h"
#include "request_handler.h"
//#include "stat_reader.h"

#include "test_framework.h"
#include "test_json.h"
#include "test_transport_catalogue.h"

#include "transport_catalogue.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
  using namespace transport;
  ::test::TestRunner::RunAllTests();

  // По-умолчанию используются std::cin и std::cout
  auto json_inputter = std::make_shared<JsonInputter>();
  auto json_outputter = std::make_shared<JsonOutputter>();

  TransportCatalogue catalog;
  MapRenderer map_render;
  RequestHandler handler(catalog, map_render);
  handler.SetInputter(json_inputter);
  handler.SetOutputter(json_outputter);
  handler.ProcessRequests();
  return 0;
}
