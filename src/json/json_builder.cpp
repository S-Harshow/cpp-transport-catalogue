#include "json_builder.h"
#include <utility>

using namespace std;

namespace json {
json::Builder::Builder() { nodes_stack_.emplace_back(&root_); }

json::KeyContext json::Builder::Key(const string &key) {

  if (nodes_stack_.empty()) {
    throw logic_error("Key without root"s);
  }
  if (nodes_stack_.back()->IsNull()) {
    throw logic_error("Key after key"s);
  }
  if (!nodes_stack_.back()->IsDict()) {
    throw logic_error("Key out of Dict"s);
  }
  if (nodes_stack_.back()->IsDict()) {
    nodes_stack_.back()->AsDict().emplace(key, Node{});
    nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(key));
  }
  return KeyContext(*this);
}

json::DictItemContext json::Builder::StartDict() {
  if (nodes_stack_.empty()) {
    throw logic_error("Dict without root"s);
  }
  const Node dict_node((Dict()));
  if (nodes_stack_.back()->IsNull()) {
    *(nodes_stack_.back()) = dict_node;
  } else if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsArray().emplace_back(dict_node);
    nodes_stack_.emplace_back(&nodes_stack_.back()->AsArray().back());
  } else {
    throw logic_error("Incorrect call of Dict starter"s);
  }
  return DictItemContext(*this);
}

json::ArrayItemContext json::Builder::StartArray() {
  if (nodes_stack_.empty()) {
    throw logic_error("Array without root"s);
  }
  const Node array_node((Array()));
  if (nodes_stack_.back()->IsNull()) {
    *(nodes_stack_.back()) = array_node;
  } else if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsArray().emplace_back(array_node);
    nodes_stack_.emplace_back(&nodes_stack_.back()->AsArray().back());
  } else {
    throw logic_error("Incorrect call of Array starter"s);
  }
  return ArrayItemContext(*this);
}

json::DictEndContext json::Builder::EndDict() {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
    throw logic_error("EndDict without StartDict"s);
  }
  //    if (nodes_stack_.size() > 1) {
  nodes_stack_.pop_back();
  //    }
  return DictEndContext(*this);
}

json::ArrayEndContext json::Builder::EndArray() {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
    throw logic_error("EndArray without StartArray"s);
  }
  //    if (nodes_stack_.size() > 1) {
  nodes_stack_.pop_back();
  //    }
  return ArrayEndContext(*this);
}

json::Node json::Builder::Build() {
  if (!nodes_stack_.empty()) {
    throw logic_error("Building with endless objects"s);
  }
  return root_;
}

json::RootValueContext json::Builder::Value(Node::Value value) {
  if (nodes_stack_.empty()) {
    throw logic_error("Value without root"s);
  }
  const Node current_node(GetNode_(move(value)));
  if (nodes_stack_.back()->IsNull()) {
    // вызов после Key
    *(nodes_stack_.back()) = current_node;
    nodes_stack_.pop_back();
    return RootValueContext(*this);
  }
  throw logic_error("Incorrect call of Value setter"s);
}

json::ArrayItemContext json::Builder::Value_array(Node::Value value) {
  if (nodes_stack_.empty()) {
    throw logic_error("Value without root"s);
  }
  const Node current_node(GetNode_(move(value)));
  if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsArray().emplace_back(current_node);
    return ArrayItemContext(*this);
  }
  throw logic_error("Incorrect call of Value setter in array"s);
}

json::DictItemContext json::Builder::Value_dict(Node::Value value) {
  if (nodes_stack_.empty()) {
    throw logic_error("Value without root"s);
  }
  const Node current_node(GetNode_(move(value)));
  if (nodes_stack_.back()->IsNull()) {
    // вызов после Key
    *(nodes_stack_.back()) = current_node;
    nodes_stack_.pop_back();
    return DictItemContext(*this);
  }
  throw logic_error("Incorrect call of Value setter in dict"s);
}

json::Node json::Builder::GetNode_(Node::Value value) {
  Node current_node;
  switch (value.index()) {
  case 0:
    current_node = get<nullptr_t>(value);
    break;
  case 1:
    current_node = move(get<Array>(value));
    break;
  case 2:
    current_node = move(get<Dict>(value));
    break;
  case 3:
    current_node = get<bool>(value);
    break;
  case 4:
    current_node = get<int>(value);
    break;
  case 5:
    current_node = get<uint>(value);
    break;
  case 6:
    current_node = get<double>(value);
    break;
  case 7:
    current_node = move(get<string>(value));
    break;
  default:
    break;
  }
  return current_node;
}

json::KeyContext::KeyContext(Builder &builder) : Context(builder) {}

json::DictItemContext json::KeyContext::Value(json::Node::Value value) {
  return Context::Value_dict_(move(value));
}

json::DictItemContext json::KeyContext::StartDict() {
  return Context::StartDict_();
}

json::ArrayItemContext json::KeyContext::StartArray() {
  return Context::StartArray_();
}

json::DictItemContext::DictItemContext(Builder &builder) : Context(builder) {}

json::KeyContext json::DictItemContext::Key(const std::string &key) {
  return Context::Key_(key);
}

json::DictEndContext json::DictItemContext::EndDict() {
  return Context::EndDict_();
}

json::ArrayItemContext::ArrayItemContext(Builder &builder) : Context(builder) {}

json::ArrayItemContext json::ArrayItemContext::Value(Node::Value value) {
  return Context::Value_array_(move(value));
}

json::DictItemContext json::ArrayItemContext::StartDict() {
  return Context::StartDict_();
}

json::ArrayItemContext json::ArrayItemContext::StartArray() {
  return Context::StartArray_();
}

json::ArrayEndContext json::ArrayItemContext::EndArray() {
  return Context::EndArray_();
}

json::Context::Context(Builder &builder) : builder_(builder) {}

json::KeyContext json::Context::Key_(const std::string &key) {
  return builder_.Key(key);
}

json::ArrayItemContext json::Context::Value_array_(Node::Value value) {
  return builder_.Value_array(move(value));
}

json::DictItemContext json::Context::Value_dict_(Node::Value value) {
  return builder_.Value_dict(move(value));
}

json::DictItemContext json::Context::StartDict_() {
  return builder_.StartDict();
}

json::ArrayItemContext json::Context::StartArray_() {
  return builder_.StartArray();
}

json::DictEndContext json::Context::EndDict_() { return builder_.EndDict(); }

json::ArrayEndContext json::Context::EndArray_() { return builder_.EndArray(); }

json::Node json::Context::Build_() { return builder_.Build(); }

json::Builder &json::Context::GetBuilder() { return builder_; }

json::RootValueContext::RootValueContext(Builder &builder) : Context(builder) {}

json::Node json::RootValueContext::Build() { return Context::Build_(); }

json::DictEndContext::DictEndContext(Builder &builder) : Context(builder) {}

json::KeyContext json::DictEndContext::Key(const string &key) {
  return Context::Key_(key);
}

json::ArrayItemContext json::DictEndContext::Value(Node::Value value) {
  return Context::Value_array_(move(value));
}

json::DictItemContext json::DictEndContext::StartDict() {
  return Context::StartDict_();
}

json::ArrayItemContext json::DictEndContext::StartArray() {
  return Context::StartArray_();
}

json::ArrayEndContext json::DictEndContext::EndArray() {
  return Context::EndArray_();
}

json::Node json::DictEndContext::Build() { return Context::Build_(); }

json::ArrayEndContext::ArrayEndContext(Builder &builder) : Context(builder) {}

json::KeyContext json::ArrayEndContext::Key(const string &key) {
  return Context::Key_(key);
}

json::ArrayItemContext json::ArrayEndContext::Value(Node::Value value) {
  return Context::Value_array_(move(value));
}

json::DictItemContext json::ArrayEndContext::StartDict() {
  return Context::StartDict_();
}

json::ArrayItemContext json::ArrayEndContext::StartArray() {
  return Context::StartArray_();
}

json::DictEndContext json::ArrayEndContext::EndDict() {
  return Context::EndDict_();
}

json::Node json::ArrayEndContext::Build() { return Context::Build_(); }
} // namespace json
