#pragma once

#include "test_framework.h"
#include "transport_catalogue.h"
#include <cassert>
#include <string>

#include <chrono>
#include <sstream>
#include <string_view>

using namespace std::literals;

namespace transport::test {
void TestByFiles(const std::string &inputFileName,
                 const std::string &responseFilePath);

} // namespace transport::test
