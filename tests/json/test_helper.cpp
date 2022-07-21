#include "test_helper.h"

using namespace testing;
using namespace std::string_literals;
using namespace json;

json::Document LoadJSON(const std::string &str) {
  std::istringstream strm(str);
  return json::Load(strm);
}

std::string Print(const json::Node &node) {
  std::ostringstream out;
  json::Print(Document{node}, out);
  return out.str();
}
