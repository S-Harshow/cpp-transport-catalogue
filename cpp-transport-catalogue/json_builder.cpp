#include "json_builder.h"

json::Builder::Builder() { nodes_stack_.emplace_back(&root_); }

json::KeyContext json::Builder::Key(const std::string &key) {

  if (nodes_stack_.empty()) {
    throw std::logic_error("Key without root");
  }
  if (nodes_stack_.back()->IsNull()) {
    throw std::logic_error("Key after key");
  }
  if (!nodes_stack_.back()->IsDict()) {
    throw std::logic_error("Key out of Dict");
  }
  if (nodes_stack_.back()->IsDict()) {
    nodes_stack_.back()->AsDict().emplace(key, Node{});
    nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(key));
  }
  return KeyContext(*this);
}

json::DictItemContext json::Builder::StartDict() {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Dict without root");
  }
  Node dict_node((Dict()));
  if (nodes_stack_.back()->IsNull()) {
    *(nodes_stack_.back()) = std::move(dict_node);
  } else if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsArray().emplace_back(std::move(dict_node));
    nodes_stack_.emplace_back(&nodes_stack_.back()->AsArray().back());
  } else {
    //    throw std::logic_error("Incorrect call of Dict starter");
  }
  return DictItemContext(*this);
}

json::ArrayItemContext json::Builder::StartArray() {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Array without root");
  }
  Node array_node((Array()));
  if (nodes_stack_.back()->IsNull()) {
    *(nodes_stack_.back()) = array_node;
  } else if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsArray().emplace_back(std::move(array_node));
    nodes_stack_.emplace_back(&nodes_stack_.back()->AsArray().back());
  } else {
    throw std::logic_error("Incorrect call of Array starter");
  }
  return ArrayItemContext(*this);
}

json::DictEndContext json::Builder::EndDict() {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
    throw std::logic_error("EndDict without StartDict");
  }
  //    if (nodes_stack_.size() > 1) {
  nodes_stack_.pop_back();
  //    }
  return DictEndContext(*this);
}

json::ArrayEndContext json::Builder::EndArray() {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
    throw std::logic_error("EndArray without StartArray");
  }
  //    if (nodes_stack_.size() > 1) {
  nodes_stack_.pop_back();
  //    }
  return ArrayEndContext(*this);
}

json::Node json::Builder::Build() {
  if (!nodes_stack_.empty()) {
    throw std::logic_error("Building with endless objects");
  }
  return root_;
}

json::RootValueContext json::Builder::Value(Node::Value value) {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Value without root");
  }
  Node current_node = GetNode_(std::move(value));
  if (nodes_stack_.back()->IsNull()) {
    // вызов после Key
    *(nodes_stack_.back()) = std::move(current_node);
    nodes_stack_.pop_back();
    return RootValueContext(*this);
  }
  throw std::logic_error("Incorrect call of Value setter");
}

json::ArrayItemContext json::Builder::Value_array(Node::Value value) {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Value without root");
  }
  Node current_node = GetNode_(std::move(value));
  if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsArray().emplace_back(current_node);
    return ArrayItemContext(*this);
  }
  throw std::logic_error("Incorrect call of Value setter in array");
}

json::DictItemContext json::Builder::Value_dict(Node::Value value) {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Value without root");
  }
  Node current_node = GetNode_(std::move(value));
  if (nodes_stack_.back()->IsNull()) {
    // вызов после Key
    *(nodes_stack_.back()) = std::move(current_node);
    nodes_stack_.pop_back();
    return DictItemContext(*this);
  }
  throw std::logic_error("Incorrect call of Value setter in dict");
}

json::Node json::Builder::GetNode_(Node::Value value) {
  Node current_node;
  switch (value.index()) {
  case 0:
    current_node = std::get<std::nullptr_t>(value);
    break;
  case 1:
    current_node = std::move(std::get<Array>(value));
    break;
  case 2:
    current_node = std::move(std::get<Dict>(value));
    break;
  case 3:
    current_node = std::get<bool>(value);
    break;
  case 4:
    current_node = std::get<int>(value);
    break;
  case 5:
    current_node = std::get<double>(value);
    break;
  case 6:
    current_node = std::move(std::get<std::string>(value));
    break;
  default:
    break;
  }
  return current_node;
}

json::KeyContext::KeyContext(Builder &builder) : Context(builder) {}

json::DictItemContext json::KeyContext::Value(json::Node::Value value) {
  return Context::Value_dict_(std::move(value));
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
  return Context::Value_array_(std::move(value));
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
  return builder_.Value_array(std::move(value));
}

json::DictItemContext json::Context::Value_dict_(Node::Value value) {
  return builder_.Value_dict(std::move(value));
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

json::DictEndContext::DictEndContext(Builder &builder) : Context(builder){}

json::KeyContext json::DictEndContext::Key(const std::string &key) { return Context::Key_(key); }

json::ArrayItemContext json::DictEndContext::Value(Node::Value value) {
  return Context::Value_array_(std::move(value));
}

json::DictItemContext json::DictEndContext::StartDict() { return Context::StartDict_(); }

json::ArrayItemContext json::DictEndContext::StartArray() { return Context::StartArray_(); }

json::ArrayEndContext json::DictEndContext::EndArray() { return Context::EndArray_(); }

json::Node json::DictEndContext::Build() { return Context::Build_(); }

json::ArrayEndContext::ArrayEndContext(Builder &builder) : Context(builder){}

json::KeyContext json::ArrayEndContext::Key(const std::string &key) { return Context::Key_(key); }

json::ArrayItemContext json::ArrayEndContext::Value(Node::Value value) {
  return Context::Value_array_(std::move(value));
}

json::DictItemContext json::ArrayEndContext::StartDict() { return Context::StartDict_(); }

json::ArrayItemContext json::ArrayEndContext::StartArray() { return Context::StartArray_(); }

json::DictEndContext json::ArrayEndContext::EndDict() { return Context::EndDict_(); }

json::Node json::ArrayEndContext::Build() { return Context::Build_(); }
