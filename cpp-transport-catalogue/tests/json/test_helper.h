#pragma once

#include "../../src/json/json.h"
#include <iostream>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

// загрузка json-документа из строки
json::Document LoadJSON(const std::string &str);

// "печать" json-документа в строку
std::string Print(const json::Node &node);

template <typename Fn> void MustThrowLogicError(Fn func) {
  ASSERT_THROW(/*void*/ func(), std::logic_error);
}
