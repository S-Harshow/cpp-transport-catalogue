#include "json_reader.h"
#include "path.h"
#include "transport_catalogue/transport_catalogue.h"
#include <iostream>
#include <vector>

#include <filesystem>
#include <fstream>

using namespace std;
using namespace filesystem;

int main() {
  using namespace transport;
  JsonOutputter::Register();
  JsonInputter::Register();

  auto json_inputter = IOFactory<Inputter>::instance().Make("json"sv, std::cin);
  auto json_outputter =
      IOFactory<Outputter>::instance().Make("json"sv, std::cout);

  RequestHandler handler{move(json_inputter), move(json_outputter)};

  //      while (true) {
  handler.Execute();
  //    }
  return 0;
}
