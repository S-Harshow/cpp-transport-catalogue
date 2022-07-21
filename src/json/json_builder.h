#pragma once

#include "json.h"

namespace json {

class Context;
// Контекст после ввода Key
class KeyContext;
// Контекст после вызова Value или StartDict
class DictItemContext;
// Контекст после закрытия словаря
class DictEndContext;
// Контекст после вызова StartArray или Value внутри массива
class ArrayItemContext;
// Контекст после закрытия массива
class ArrayEndContext;
// Контекст после одиночного корневого элемента
class RootValueContext;

class Builder {
  friend Context;

public:
  Builder();
  // Начинает определение сложного значения-словаря.
  DictItemContext StartDict();

  // Начинает определение сложного значения-массива.
  ArrayItemContext StartArray();

  // Задаёт значение, если вызвать сразу после конструктора json::Builder, всё
  // содержимое конструируемого JSON-объекта.
  RootValueContext Value(Node::Value value);

protected:
  // При определении словаря задаёт строковое значение ключа для очередной пары
  // ключ-значение.
  KeyContext Key(const std::string &key);

  // Задаёт значение, соответствующее ключу при определении словаря.
  ArrayItemContext Value_array(Node::Value value);

  // Задаёт значение очередной элемент массива.
  DictItemContext Value_dict(Node::Value value);

  // Завершает определение сложного значения-словаря.
  DictEndContext EndDict();

  // Завершает определение сложного значения-массива.
  ArrayEndContext EndArray();

  // Возвращает объект json::Node, содержащий JSON, описанный предыдущими
  // вызовами методов.
  Node Build();

private:
  // конструируемый объект
  Node root_{};

  // стек указателей на те вершины JSON, которые ещё не построены: то есть
  // текущее описываемое значение и цепочка его родителей. Он поможет
  // возвращаться в нужный контекст после вызова End-методов.
  std::vector<Node *> nodes_stack_{};
  static Node GetNode_(Node::Value value);
};

class Context {

public:
  explicit Context(Builder &builder);
  Context(const Context &other) = delete;
  Context(Context &&other) = default;
  Context &operator=(const Context &other) = delete;
  Context &operator=(Context &&other) = delete;
  virtual ~Context()= default;

protected:
  virtual KeyContext Key_(const std::string &key);
  virtual ArrayItemContext Value_array_(Node::Value value);
  virtual DictItemContext Value_dict_(Node::Value value);
  virtual DictItemContext StartDict_();
  virtual ArrayItemContext StartArray_();
  virtual DictEndContext EndDict_();
  virtual ArrayEndContext EndArray_();
  virtual Node Build_();
  Builder &GetBuilder();

private:
  Builder &builder_;
};

class KeyContext final : private Context {
public:
  explicit KeyContext(Builder &builder);
  DictItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
};

class DictItemContext final : private Context {
public:
  explicit DictItemContext(Builder &builder);
  KeyContext Key(const std::string &key);
  DictEndContext EndDict();
};

class ArrayItemContext final : private Context {
public:
  explicit ArrayItemContext(Builder &builder);
  ArrayItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
  ArrayEndContext EndArray();
};

class RootValueContext final : private Context {
public:
  explicit RootValueContext(Builder &builder);
  virtual Node Build();
};

class DictEndContext final : private Context {
public:
  explicit DictEndContext(Builder &builder);
  KeyContext Key(const std::string &key);
  ArrayItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
  ArrayEndContext EndArray();
  virtual Node Build();
};

class ArrayEndContext final : private Context {
public:
  explicit ArrayEndContext(Builder &builder);
  KeyContext Key(const std::string &key);
  ArrayItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
  DictEndContext EndDict();
  virtual Node Build();
};

} // namespace json
