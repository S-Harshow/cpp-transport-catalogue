#pragma once

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#define TEST_CLASS_NAME_(module_name, test_name)                               \
  module_name##_##test_name##_Test

#define TEST_IMPL_(module_name, test_name, test_class_name, runs, iterations)  \
  class TEST_CLASS_NAME_(module_name, test_name) final                         \
      : public test_class_name {                                               \
  public:                                                                      \
    TEST_CLASS_NAME_(module_name, test_name)() = default;                      \
                                                                               \
  protected:                                                                   \
    void TestBody() override;                                                  \
                                                                               \
  private:                                                                     \
    static const std::shared_ptr<::test::detail::TestDescriptor> _descriptor;  \
  };                                                                           \
                                                                               \
  const std::shared_ptr<::test::detail::TestDescriptor> TEST_CLASS_NAME_(      \
      module_name, test_name)::_descriptor =                                   \
      ::test::TestRunner::Instance().RegisterTest(                             \
          #module_name, #test_name, GetType(), runs, iterations,               \
          std::make_shared<::test::detail::TestFactoryDefault<                 \
              TEST_CLASS_NAME_(module_name, test_name)>>());                   \
                                                                               \
  void TEST_CLASS_NAME_(module_name, test_name)::TestBody()

#define TESTCASE(module_name, test_name)                                       \
  TEST_IMPL_(module_name, test_name, ::test::detail::TestCase, 0, 0)

#define BENCHMARK(module_name, test_name, runs, iterations)                    \
  TEST_IMPL_(module_name, test_name, ::test::detail::Benchmark, runs,          \
             iterations)

namespace test::detail {
using namespace std::string_view_literals;

constexpr int OUTPUT_PRECISION = 5;
constexpr int OUTPUT_HALF_PRECISION = OUTPUT_PRECISION / 2;
constexpr double OUTPUT_KILO = 1000.;
constexpr double OUTPUT_MEGA = 1000000.;
constexpr double OUTPUT_GIGA = 1000000000.;

// Console text colors.
enum class Console {
  TextDefault, // Default console color. Used for resets.
  TextBlack,   // Black
  TextBlue,    // Blue
  TextGreen,   // Green
  TextCyan,    // Cyan
  TextRed,     // Red
  TextPurple,  // Purple
  TextYellow,  // Yellow
  TextWhite    // White
};

inline std::ostream &operator<<(std::ostream &stream, Console color) {
  if (((stream.rdbuf() != std::cout.rdbuf()) &&
       (stream.rdbuf() != std::cerr.rdbuf()))) {
    return stream;
  }
  switch (color) {
  case Console::TextDefault:
    stream << "\033[m"sv;
    break;
  case Console::TextBlack:
    stream << "\033[0;30m"sv;
    break;
  case Console::TextBlue:
    stream << "\033[0;34m"sv;
    break;
  case Console::TextGreen:
    stream << "\033[0;32m"sv;
    break;
  case Console::TextCyan:
    stream << "\033[0;36m"sv;
    break;
  case Console::TextRed:
    stream << "\033[0;31m"sv;
    break;
  case Console::TextPurple:
    stream << "\033[0;35m"sv;
    break;
  case Console::TextYellow:
    stream << "\033[0;33m"sv;
    break;
  case Console::TextWhite:
    stream << "\033[0;37m"sv;
    break;
  }
  return stream;
}
struct TestsResult;
class TestDescriptor;
class Test {
public:
  Test() = default;
  Test(const Test &other) = default;
  Test &operator=(const Test &other) = default;
  Test(Test &&other) = default;
  Test &operator=(Test &&other) = default;
  virtual ~Test() = default;
  using RunResult = std::pair<uint64_t, std::string>;

  // Подготовка тестового окружения
  virtual void Init();

  // Очистка окружения после теста
  virtual void CleanUp();

  // Запуск теста
  virtual RunResult Run(std::size_t iterations) = 0;

protected:
  // Тело тестовой функции. Выполняется на каждой итерации бенчмарка.
  virtual void TestBody() = 0;
};

class TestCase : public Test {
public:
  // Запуск теста
  Test::RunResult Run(std::size_t iterations = 1) override;
  static size_t GetType() { return __LINE__; };
};

class Benchmark : public Test {
public:
  // Запуск теста
  Test::RunResult Run(std::size_t iterations) override;
  static size_t GetType() { return __LINE__; };
};

class TestFactory {
public:
  TestFactory() = default;
  TestFactory(const TestFactory &other) = default;
  TestFactory &operator=(const TestFactory &other) = default;
  TestFactory(TestFactory &&other) = default;
  TestFactory &operator=(TestFactory &&other) = default;
  // Виртуальный декстурктор
  virtual ~TestFactory() = default;

  // Создать класс-тестировщик
  virtual std::shared_ptr<Test> CreateTest() = 0;
  // Тип теста
  virtual size_t GetTypeCode() = 0;
};

template <class T> class TestFactoryDefault : public TestFactory {
public:
  std::shared_ptr<Test> CreateTest() noexcept override {
    return std::make_shared<T>();
  }
  size_t GetTypeCode() override { return typeid(T).hash_code(); };
};

// Характеристики теста.
class TestDescriptor {
public:
  //  TestDescriptor(const TestDescriptor &other) = default;
  //  TestDescriptor &operator=(const TestDescriptor &other) = default;
  //  TestDescriptor(TestDescriptor &&other) = default;
  //  TestDescriptor &operator=(TestDescriptor &&other) = default;
  //  ~TestDescriptor();

  // Создаёт новый объект с характеристиками теста
  TestDescriptor(const std::string &module_name, const std::string &test_name,
                 size_t test_type, std::size_t runs, std::size_t iterations,
                 std::shared_ptr<TestFactory> test_factory,
                 bool is_disabled = false);
  // Имя модуля
  [[nodiscard]] const std::string &getModuleName() const;
  // Имя теста
  [[nodiscard]] const std::string &getTestName() const;
  // Каноническое имя: <ModuleName>.<TestName>
  [[nodiscard]] std::string getCanonicalName() const;
  // Количество запусков теста
  [[nodiscard]] std::size_t getRuns() const;
  // Колличество итераций за один запуск теста
  [[nodiscard]] std::size_t getIterations() const;
  // Паттерн фабрики теста
  std::weak_ptr<TestFactory> getFactory();
  // Признак отмены выполнения теста
  [[nodiscard]] bool IsDisabled() const;
  // Тип теста
  [[nodiscard]] size_t GetType() const;

private:
  std::string module_name_{};
  std::string test_name_{};
  size_t test_type_;
  std::size_t runs_{};
  std::size_t iterations_{};
  bool is_disabled_{};
  std::shared_ptr<TestFactory> factory_{};
};

// Все интерваля в наносекундах
struct TestsResult {
public:
  // Инициализация вектором результатов
  TestsResult(size_t test_type, const std::string &error_text,
              const std::vector<uint64_t> &run_times = {},
              std::size_t iterations = 0);

  // Общее время тестирования
  [[nodiscard]] double TimeTotal() const;

  // Количество запусков
  [[nodiscard]] inline const std::vector<uint64_t> &RunTimes() const {
    return run_times_;
  }

  // Среднее время за запуск
  [[nodiscard]] inline double RunTimeAverage() const {
    return double(time_total_) / double(run_times_.size());
  }

  // Максимально время за запуск
  [[nodiscard]] inline double RunTimeMaximum() const {
    return double(time_run_max_);
  }

  // Минимальное время за запуск
  [[nodiscard]] inline double RunTimeMinimum() const {
    return double(time_run_min_);
  }

  // Среднее время на итерацию
  [[nodiscard]] inline double IterationTimeAverage() const {
    return RunTimeAverage() / double(iterations_);
  }

  // Минимальное время на итерацию
  [[nodiscard]] inline double IterationTimeMinimum() const {
    return double(time_run_min_) / double(iterations_);
  }

  // Максимальное время на итерацию
  [[nodiscard]] inline double IterationTimeMaximum() const {
    return double(time_run_max_) / double(iterations_);
  }

  // Среднее колличество итераций за секунду
  [[nodiscard]] inline double IterationsPerSecondAverage() const {
    return OUTPUT_GIGA / IterationTimeAverage();
  }

  // Минимальное количество итераций за секунду
  [[nodiscard]] inline double IterationsPerSecondMinimum() const {
    return OUTPUT_GIGA / IterationTimeMaximum();
  }

  // Максимально количество итераций за секунду
  [[nodiscard]] inline double IterationsPerSecondMaximum() const {
    return OUTPUT_GIGA / IterationTimeMinimum();
  }

  // Среднее количество запусков в секунду
  [[nodiscard]] inline double RunsPerSecondAverage() const {
    return OUTPUT_GIGA / RunTimeAverage();
  }
  // Текст сообщения об ошибке
  [[nodiscard]] inline std::string ErrorMessage() const { return error_text_; }

  // Признак наличия ошибки в результатах
  [[nodiscard]] inline bool IsError() const { return !error_text_.empty(); }

  // Вернуть тип теста
  [[nodiscard]] inline size_t GetType() const { return test_type_; }

private:
  size_t test_type_{};
  std::string error_text_{};
  std::vector<uint64_t> run_times_{};
  std::size_t iterations_{};
  uint64_t time_total_{};
  uint64_t time_run_min_{};
  uint64_t time_run_max_{};
};

// Выводильщик
// Абстрактный базовый класс.
class Outputter {
public:
  Outputter() = default;
  Outputter(const Outputter &other) = delete;
  Outputter &operator=(const Outputter &other) = delete;
  Outputter(Outputter &&other) = default;
  Outputter &operator=(Outputter &&other) = default;
  virtual ~Outputter() = default;

  // Информирование о начале тестирования
  virtual void Begin(const std::size_t &enabled_count,
                     const std::size_t &disabled_count) = 0;

  // Информирование об окончании тестирования
  virtual void End(const std::size_t &executed_count,
                   const std::size_t &failed_count,
                   const std::size_t &disabled_count) = 0;
  // Информирование о начале выполнения теста
  virtual void BeginTest(const std::string &module_name,
                         const std::string &test_name, size_t test_type,
                         const std::size_t &runs_count = 0,
                         const std::size_t &iterations_count = 0) = 0;

  // Информирование о результатах выполнения теста
  virtual void EndTest(const std::string &module_name,
                       const std::string &test_name,
                       const TestsResult &result) = 0;

  // Информирование о пропущеном тесте
  virtual void SkipDisabledTest(const std::string &module_name,
                                const std::string &test_name,
                                const std::size_t &runs_count,
                                const std::size_t &iterations_count) = 0;

protected:
  // Форматирует заголовок теста
  static void WriteTestNameToStream(std::ostream &stream,
                                    const std::string &moduleName,
                                    const std::string &testName);
};

// Выводильщик в консоль
// Выводит результаты тестирования в стандартный поток вывода
class ConsoleOutputter final : public Outputter {
public:
  // Конструктор
  explicit ConsoleOutputter(std::ostream &stream = std::cout);
  // Реализация интрефейса Outputter
  void Begin(const std::size_t &enabled_count,
             const std::size_t &disabled_count) override;
  void End(const std::size_t &executed_count, const std::size_t &failed_count,
           const std::size_t &disabled_count) override;

  void BeginTest(const std::string &module_name, const std::string &test_name,
                 size_t test_type, const std::size_t &runs_count = 0,
                 const std::size_t &iterations_count = 0) override;
  void SkipDisabledTest(const std::string &module_name,
                        const std::string &test_name,
                        const std::size_t &runs_count,
                        const std::size_t &iterations_count) override;
  void EndTest(const std::string &module_name, const std::string &test_name,
               const TestsResult &result) override;

private:
  void BeginOrSkipTest(const std::string &module_name,
                       const std::string &test_name, size_t test_type,
                       std::size_t runs_count, std::size_t iterations_count,
                       bool skip);
  std::ostream &stream_;
};

// Тестировщик (singleton)
class TestRunner {
public:
  TestRunner(const TestRunner &other) = delete;
  TestRunner &operator=(const TestRunner &other) = delete;

  // Get the singleton instance of TestRunner
  static TestRunner &Instance();

  // Регистрирует тесты в тестировщике
  static std::shared_ptr<TestDescriptor>
  RegisterTest(const std::string &module_name, const std::string &test_name,
               size_t test_type, std::size_t runs, std::size_t iterations,
               const std::shared_ptr<TestFactory> &test_factory);

  // Добавить выводильщика
  static void AddOutputter(Outputter &outputter);

  // Запустить все тесты
  static void RunAllTests();

  // Перечислить все тесты
  static std::vector<const TestDescriptor *> ListTests();

  // Перетасовать все тесты
  static void ShuffleTests();

private:
  TestRunner() = default;
  TestRunner(TestRunner &&other) = default;
  TestRunner &operator=(TestRunner &&other) = default;
  ~TestRunner() = default;

  // Получить тесты на запуск
  [[nodiscard]] std::vector<std::shared_ptr<TestDescriptor>> GetTests() const;

  // Тестирование по типам тестировщиков
  [[nodiscard]] static std::shared_ptr<TestsResult>
  TestCase_(const std::shared_ptr<TestDescriptor> &descriptor);
  [[nodiscard]] static std::shared_ptr<TestsResult>
  Benchmark_(const std::shared_ptr<TestDescriptor> &descriptor);

  std::vector<Outputter *> outputters_; // Зарегистрированные выводильщики
  std::vector<std::shared_ptr<TestDescriptor>>
      tests_; // Зарегистрированные тесты
};

template <class Map>
std::ostream &PrintMap(std::ostream &stream, const Map &map_) {
  stream << "{";
  bool first = true;
  for (const auto &key_value : map_) {
    if (!first) {
      stream << ", ";
    }
    first = false;
    stream << key_value.first << ": " << key_value.second;
  }
  return stream << "}";
}

template <class Set>
std::ostream &PrintSet(std::ostream &stream, const Set &set_) {
  stream << "{";
  bool first = true;
  for (const auto &item : set_) {
    if (!first) {
      stream << ", ";
    }
    first = false;
    stream << item;
  }
  return stream << "}";
}
} // namespace test::detail
namespace test {
using TestRunner = test::detail::TestRunner;
} // namespace test

template <class T, class C>
std::ostream &operator<<(std::ostream &stream, const std::set<T, C> &set_) {
  return PrintSet(stream, set_);
}

template <class T, class H, class Eq>
std::ostream &operator<<(std::ostream &stream,
                         const std::unordered_set<T, H, Eq> &un_set_) {
  return PrintSet(stream, un_set_);
}

template <class K, class V, class C>
std::ostream &operator<<(std::ostream &stream, const std::map<K, V, C> &map_) {
  return PrintMap(stream, map_);
}

template <class K, class V, class H, class Eq>
std::ostream &operator<<(std::ostream &stream,
                         const std::unordered_map<K, V, H, Eq> &un_map_) {
  return PrintMap(stream, un_map_);
}

/**
 * Сравнивает значения t и u. Если они не равны, тест проваливается.
 * Строка hint содержит подсказку, которая выводится, если тест провален.
 *
 * Пример:
 *  void Test() {
 *      Assert("Hello "s + "world"s, "Hello world"s, "String concatenation
 * error"s);
 *  }
 */
template <class T, class U>
void AssertEqual(const T &lhv, const U &rhv, const std::string &hint = {}) {
  if (!(lhv == rhv)) {
    std::ostringstream stream;
    stream << "Assertion failed: " << lhv << " != " << rhv;
    if (!hint.empty()) {
      stream << " Hint: " << hint;
    }
    throw std::runtime_error(stream.str());
  }
}

/**
 * Сравнивает значения t и u. Если они равны, тест проваливается.
 * Строка hint содержит подсказку, которая выводится, если тест провален.
 *
 * Пример:
 *  void Test() {
 *      AssertNotEqual("Hello "s + "world"s, "Hello my world"s, "String
 * concatenation error"s);
 *  }
 */
template <class T, class U>
void AssertNotEqual(const T &lhv, const U &rhv, const std::string &hint = {}) {
  if (lhv == rhv) {
    std::ostringstream stream;
    stream << "Assertion failed: " << lhv << " == " << rhv;
    if (!hint.empty()) {
      stream << " Hint: " << hint;
    }
    throw std::runtime_error(stream.str());
  }
}

/**
 * Проверяет истинность значения b, если нет, тест проваливается.
 * Строка hint содержит подсказку, которая выводится, если тест провален.
 */
inline void Assert(bool lhv, const std::string &hint) {
  AssertEqual(lhv, true, hint);
}

/**
 * Класс TestRunner запускает тест-функции.
 * Пример:
 *  void Test1() {
 *      // ...
 *  }
 *
 *  void Test2() {
 *      // ...
 *  }
 *
 *  int main() {
 *      TestRunner tr;
 *      // Запускает функцию Test1. Если тест будет провален, его имя будет
 * выведено как
 *      // First test
 *      tr.RunTest(Test1, "First test"s);
 *      // Если имя теста, совпадает с именем тест-функции, можно использовать
 * максро RUN_TEST: RUN_TEST(tr, Test2); // Аналогично tr.RunTest(Test2,
 * "Test2");
 *  }
 */

#ifndef FILE_NAME
#define FILE_NAME __FILE__
#endif

/**
 * Макрос ASSERT_EQUAL проверяет значения выражений x и y на равенство.
 * Если значения не равны, тест считается проваленным.
 *
 * Пример:
 *  void Test() {
 *      ASSERT_EQUAL(2 + 2, 4);
 *      ASSERT_EQUAL(2 + 2, 5); // Эта проверка не сработает, и тест будет
 * провален
 *  }
 */
#define ASSERT_EQUAL(x, y)                                                     \
  {                                                                            \
    std::ostringstream __assert_equal_private_os;                              \
    __assert_equal_private_os << #x << " != " << #y                            \
                              << " Location: " << FILE_NAME << ":"             \
                              << __LINE__;                                     \
    AssertEqual(x, y, __assert_equal_private_os.str());                        \
  }

/**
 * Макрос ASSERT_NOT_EQUAL проверяет значения выражений x и y на не равенство.
 * Если значения равны, тест считается проваленным.
 *
 * Пример:
 *  void Test() {
 *      ASSERT_EQUAL(2 + 2, 5);
 *      ASSERT_EQUAL(2 + 2, 4); // Эта проверка не сработает, и тест будет
 * провален
 *  }
 */
#define ASSERT_NOT_EQUAL(x, y)                                                 \
  {                                                                            \
    std::ostringstream __assert_equal_private_os;                              \
    __assert_equal_private_os << #x << " == " << #y                            \
                              << " Location: " << FILE_NAME << ":"             \
                              << __LINE__;                                     \
    AssertNotEqual(x, y, __assert_equal_private_os.str());                     \
  }

/**
 * Макрос ASSERT проверяет истинность выражения x. Выражение x должно
 * конвертироваться к типу bool.
 * Если выражение x ложно, тест считается проваленным. Если выражение x истинно,
 * выполнение теста продолжается.
 *
 * Пример:
 *  void Test() {
 *      ASSERT(2 + 2 == 4);
 *      ASSERT(2); // число 2 при преобразовании к bool станет значением true
 *      ASSERT(false); // здесь тест провалится
 *      string user_name = "Harry Potter"s;
 *      // Если раскомментировать следующую строку, программа не скомпилируется,
 *      // так как string не может быть преобразован к типу bool.
 *      // ASSERT(user_name);
 *  }
 */
#define ASSERT(x)                                                              \
  {                                                                            \
    std::ostringstream __assert_private_os;                                    \
    __assert_private_os << #x << " is false Location: " << FILE_NAME << ":"    \
                        << __LINE__;                                           \
    Assert(static_cast<bool>(x), __assert_private_os.str());                   \
  }

/**
 * Макрос RUN_TEST служит для удобного запуска тест-функции func.
 * Параметр tr задаёт имя переменной типа TestRunner.
 *
 * Пример:
 *  void Test1() {
 *      // Содержимое тест-функции ...
 *  }
 *
 *  void Test2() {
 *      // Содержимое тест-функции ...
 *  }
 *
 *  int main() {
 *      TestRunner tr;
 *      RUN_TEST(tr, Test1);
 *      RUN_TEST(tr, Test2);
 *  }
 */
//#define RUN_TEST(tr, func) tr.RunTest(func, #func)

/**
 * Макрос ASSERT_THROWS проверяет, что при вычислении выражения expr будет
 * выброшено исключение типа expected_exception.
 * Если исключение выброшено не будет, либо выбросится исключение другого типа,
 * тест считается проваленным.
 *
 * Пример:
 *  void Test() {
 *      using namespace std;
 *      ASSERT_THROWS(stoi("not-a-number"s), invalid_argument);
 *  }
 */
/*
#define ASSERT_THROWS(expr, expected_exception)                                \
  {                                                                            \
    bool __assert_private_flag = true;                                         \
    try {                                                                      \
      expr;                                                                    \
      __assert_private_flag = false;                                           \
    } catch (expected_exception &) {                                           \
    } catch (...) {                                                            \
      std::ostringstream __assert_private_os;                                  \
      __assert_private_os << "Expression " #expr                               \
                             " threw an unexpected exception"                  \
                             " " FILE_NAME ":"                                 \
                          << __LINE__;                                         \
      Assert(false, __assert_private_os.str());                                \
    }                                                                          \
    if (!__assert_private_flag) {                                              \
      std::ostringstream __assert_private_os;                                  \
      __assert_private_os << "Expression " #expr                               \
                             " is expected to throw " #expected_exception      \
                             " " FILE_NAME ":"                                 \
                          << __LINE__;                                         \
      Assert(false, __assert_private_os.str());                                \
    }                                                                          \
  }
*/
/**
 * Макрос ASSERT_DOESNT_THROW проверяет, что при вычислении выражения expr
 * не будет выброшено никаких исключений.
 * Если при вычислении выражения expr выбросится исключение, тест будет
 * провален.
 *
 * Пример:
 *  void Test() {
 *      vector<int> v;
 *      v.push_back(1);
 *      ASSERT_DOESNT_THROW(v.at(0)));
 *  }
 */
/*
#define ASSERT_DOESNT_THROW(expr)                                              \
  try {                                                                        \
    expr;                                                                      \
  } catch (...) {                                                              \
    std::ostringstream __assert_private_os;                                    \
    __assert_private_os << "Expression " #expr                                 \
                           " threw an unexpected exception"                    \
                           " " FILE_NAME ":"                                   \
                        << __LINE__;                                           \
    Assert(false, __assert_private_os.str());                                  \
  }
*/
