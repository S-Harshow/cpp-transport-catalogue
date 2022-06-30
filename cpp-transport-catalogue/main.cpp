#include "request_handler.h"
#include "transport_catalogue.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
  using namespace transport;

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
