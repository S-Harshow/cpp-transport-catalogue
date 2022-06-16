#pragma once

#include "json.h"
#include "test_framework.h"
#include <cassert>
#include <string>

#include <chrono>
#include <sstream>
#include <string_view>

using namespace std::literals;

namespace json::test {
json::Document LoadJSON(const std::string &str);

// "печать" json-документа в строку
std::string Print(const Node &node);

void MustFailToLoad(const std::string &str);

template <typename Fn> void MustThrowLogicError(Fn func) {
  try {
    /*void*/ func();
    std::cerr << "logic_error is expected"sv << std::endl;
    assert(false);
  } catch (const std::logic_error &) {
    // ok
  } catch (const std::exception &e) {
    std::cerr << "exception thrown: "sv << e.what() << std::endl;
    assert(false);
  } catch (...) {
    std::cerr << "Unexpected error"sv << std::endl;
    assert(false);
  }
}

} // namespace json::test
