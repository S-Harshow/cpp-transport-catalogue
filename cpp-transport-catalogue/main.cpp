#include "input_reader.h"
#include "stat_reader.h"
#include "test_example_functions.h"
#include "transport_catalogue.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
  using namespace transport;
  //  tests::TestTransportCatalogue();
  vector<io::Query> query = io::stream::read(cin);
  TransportCatalogue catalog;
  fillCatalogue(&catalog, query);

  vector<io::Query> requests = io::stream::read(cin);
  auto responses = executeRequests(&catalog, requests);
  cout << (*responses) << endl;
  return 0;
}
