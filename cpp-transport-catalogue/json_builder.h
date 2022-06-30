#pragma once

#include "json.h"

namespace json {

class Context;
// правило 1.Непосредственно после Key вызван не Value, не StartDict и не
// StartArray.
class KeyContext;

// правило 2.После вызова Value, последовавшего за вызовом Key, вызван не Key и
// не EndDict.
// правило 3.За вызовом StartDict следует не Key и не EndDict.
class DictItemContext;
class DictEndContext;
// правило 4.За вызовом StartArray следует не Value, не StartDict, не StartArray
// и не EndArray.
// правило 5.После вызова StartArray и серии Value следует не Value, не
// StartDict, не StartArray и не EndArray.
class ArrayItemContext;
class ArrayEndContext;
class RootValueContext;

class Builder {
  friend Context;

public:
  Builder();
  /* StartDict(). Начинает определение сложного значения-словаря. Вызывается в
    тех же контекстах, что и Value. Следующим вызовом обязательно должен быть
    Key или EndDict.
    Вызов исключения std::logic_error при вызове StartDict где-либо, кроме как
    после конструктора, после Key или после предыдущего элемента массива.*/
  DictItemContext StartDict();

  /* StartArray(). Начинает определение сложного значения-массива. Вызывается в
    тех же контекстах, что и Value. Следующим вызовом обязательно должен быть
    EndArray или любой, задающий новое значение: Value, StartDict или
    StartArray.
    Вызов исключения std::logic_error при вызове StartArray где-либо, кроме как
    после конструктора, после Key или после предыдущего элемента массива.*/
  ArrayItemContext StartArray();

  RootValueContext Value(Node::Value value);

protected:
  /* Key(std::string). При определении словаря задаёт строковое значение ключа
  для очередной пары ключ-значение. Следующий вызов метода обязательно должен
  задавать соответствующее этому ключу значение с помощью метода Value или
  начинать его определение с помощью StartDict или StartArray.
  Вызов исключения std::logic_error при вызове метода Key снаружи словаря или
  сразу после другого Key. */
  KeyContext Key(const std::string &key);

  /* Value(Node::Value). Задаёт значение, соответствующее ключу при определении
    словаря, очередной элемент массива или, если вызвать сразу после
    конструктора json::Builder, всё содержимое конструируемого JSON-объекта.
    Может принимать как простой объект — число или строку — так и целый массив
    или словарь. Здесь Node::Value — это синоним для базового класса Node,
    шаблона variant с набором возможных типов-значений. Смотрите заготовку кода.
Вызов исключения std::logic_error при вызове Value где-либо, кроме как после
конструктора, после Key или после предыдущего элемента массива.*/

  ArrayItemContext Value_array(Node::Value value);
  DictItemContext Value_dict(Node::Value value);

  /* EndDict(). Завершает определение сложного значения-словаря. Последним
    незавершённым вызовом Start* должен быть StartDict.
    Вызов исключения std::logic_error при вызове EndArray в контексте другого
    контейнера */
  DictEndContext EndDict();

  /* EndArray(). Завершает определение сложного значения-массива. Последним
    незавершённым вызовом Start* должен быть StartArray.
    Вызов исключения std::logic_error при вызове EndArray в контексте другого
    контейнера */
  ArrayEndContext EndArray();

  /* Build(). Возвращает объект json::Node, содержащий
    JSON, описанный предыдущими вызовами методов. К этому моменту для каждого
    Start* должен быть вызван соответствующий End*. При этом сам объект должен
    быть определён, то есть вызов json::Builder{}.Build() недопустим.
    Вызов исключения std::logic_error при вызове метода Build при неготовом
    описываемом объекте, то есть сразу после конструктора или при
    незаконченных массивах и словарях. */
  Node Build();
  /*
    Возвращаемое значение каждого метода, кроме Build, должно быть Builder&.
    */

private:
  // конструируемый объект
  Node root_{};

  std::string key_{};
  // стек указателей на те вершины JSON, которые ещё не построены: то есть
  // текущее описываемое значение и цепочка его родителей. Он поможет
  // возвращаться в нужный контекст после вызова End-методов.
  std::vector<Node *> nodes_stack_{};
  Node GetNode_(Node::Value value);
};

class Context {

public:
  explicit Context(Builder &builder);
  //  Context(const Context &other) = delete;
  //  Context &operator=(const Context &other) = delete;

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
  ~Context() = default;

private:
  Builder &builder_;
};

// правило 1.Непосредственно после Key вызван не Value, не StartDict и не
// StartArray.
class KeyContext final : private Context {
public:
  explicit KeyContext(Builder &builder);
  DictItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
};

// правило 2.После вызова Value, последовавшего за вызовом Key, вызван не Key и
// не EndDict.
// правило 3.За вызовом StartDict следует не Key и не EndDict.
class DictItemContext final : private Context {
public:
  explicit DictItemContext(Builder &builder);
  KeyContext Key(const std::string &key);
  DictEndContext EndDict();
};

// правило 4.За вызовом StartArray следует не Value, не StartDict, не StartArray
// и не EndArray.
// правило 5.После вызова StartArray и серии Value следует не Value, не
// StartDict, не StartArray и не EndArray.
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
