#include "test_framework.h"

#include <utility>

#define PAD(x) stream_ << std::setw(34) << x << std::endl;
#define PAD_DEVIATION(description, deviated, average, unit)                    \
  {                                                                            \
    double _d_ = double(deviated) - double(average);                           \
                                                                               \
    PAD(description << deviated << " " << unit << " ("                         \
                    << (deviated < average ? Console::TextRed                  \
                                           : Console::TextGreen)               \
                    << (deviated > average ? "+" : "") << _d_ << " " << unit   \
                    << " / " << (deviated > average ? "+" : "")                \
                    << (_d_ * 100.0 / average) << " %" << Console::TextDefault \
                    << ")");                                                   \
  }

#define PAD_DEVIATION_INVERSE(description, deviated, average, unit)            \
  {                                                                            \
    double _d_ = double(deviated) - double(average);                           \
                                                                               \
    PAD(description << deviated << " " << unit << " ("                         \
                    << (deviated > average ? Console::TextRed                  \
                                           : Console::TextGreen)               \
                    << (deviated > average ? "+" : "") << _d_ << " " << unit   \
                    << " / " << (deviated > average ? "+" : "")                \
                    << (_d_ * 100.0 / average) << " %" << Console::TextDefault \
                    << ")");                                                   \
  }

using namespace std;

namespace test::detail {

/*------------------------ TestDescriptor ----------------------------------*/

TestDescriptor::TestDescriptor(const std::string &module_name,
                               const std::string &test_name, size_t test_type,
                               std::size_t runs, std::size_t iterations,
                               std::shared_ptr<TestFactory> test_factory,
                               bool is_disabled)
    : module_name_(module_name), test_name_(test_name), test_type_(test_type),
      runs_(runs), iterations_(iterations), is_disabled_(is_disabled),
      factory_(std::move(test_factory)) {}

bool TestDescriptor::IsDisabled() const { return is_disabled_; }

size_t TestDescriptor::GetType() const { return test_type_; }

const std::string &TestDescriptor::getTestName() const { return test_name_; }

string TestDescriptor::getCanonicalName() const {
  return module_name_ + "."s + test_name_;
}

std::size_t TestDescriptor::getRuns() const { return runs_; }

std::size_t TestDescriptor::getIterations() const { return iterations_; }

std::weak_ptr<TestFactory> TestDescriptor::getFactory() {
  return weak_ptr<TestFactory>(factory_);
}

const std::string &TestDescriptor::getModuleName() const {
  return module_name_;
}

/*-------------------------- TestsResult ------------------------------------*/
TestsResult::TestsResult(size_t test_type, const std::string &error_text,
                         const std::vector<uint64_t> &run_times,
                         std::size_t iterations)
    : test_type_(test_type), error_text_(error_text), run_times_(run_times),
      iterations_(iterations) {
  // Summarize under the assumption of values being accessed more
  // than once.
  if (!run_times.empty()) {
    time_total_ = std::accumulate(run_times_.begin(), run_times_.end(), 0UL);
    const auto [time_min, time_max] =
        std::minmax_element(run_times_.begin(), run_times_.end());
    time_run_max_ = *time_max;
    time_run_min_ = *time_min;
  }
}

double TestsResult::TimeTotal() const { return double(time_total_); }

/*-------------------------- TestRunner --------------------------------------*/
TestRunner &TestRunner::Instance() {
  static TestRunner singleton;
  return singleton;
}

std::shared_ptr<TestDescriptor>
TestRunner::RegisterTest(const std::string &module_name,
                         const std::string &test_name, size_t test_type,
                         std::size_t runs, std::size_t iterations,
                         const std::shared_ptr<TestFactory> &test_factory) {
  // Определить является ли тест отключенным.
  static const char *disabledPrefix = "DISABLED_";
  static const int disabledPrefix_size = 9;
  bool is_disabled =
      ((test_name.size() >= disabledPrefix_size) &&
       (test_name.substr(0, disabledPrefix_size) == string(disabledPrefix)));

  const std::string real_test_name =
      (is_disabled) ? test_name.substr(disabledPrefix_size) : test_name;

  // Создать описание теста
  shared_ptr<TestDescriptor> descriptor =
      make_unique<TestDescriptor>(module_name, real_test_name, test_type, runs,
                                  iterations, test_factory, is_disabled);
  // Добавить описание в коллецию тестов
  Instance().tests_.push_back(descriptor);

  return descriptor;
}

void TestRunner::AddOutputter(Outputter &outputter) {
  Instance().outputters_.push_back(&outputter);
}

void TestRunner::RunAllTests() {
  ConsoleOutputter defaultOutputter;
  std::vector<Outputter *> defaultOutputters;
  defaultOutputters.push_back(&defaultOutputter);

  TestRunner &instance = Instance();
  vector<Outputter *> &outputters =
      (instance.outputters_.empty() ? defaultOutputters : instance.outputters_);

  // Получить все тесты на исполнение
  vector<shared_ptr<TestDescriptor>> tests = instance.GetTests();

  const size_t total_count = tests.size();
  size_t disabled_count = 0;
  size_t failed_count = 0;

  // TOFIX исправить на алгоритм
  auto testsIt = tests.begin();

  while (testsIt != tests.end()) {
    if ((*testsIt)->IsDisabled()) {
      ++disabled_count;
    }
    ++testsIt;
  }
  // алгоритм

  const size_t enabledCount = total_count - disabled_count;

  // Вывести информацию о начале тестирования
  for_each(outputters.begin(), outputters.end(),
           [&enabledCount, &disabled_count](Outputter *outputter) {
             outputter->Begin(enabledCount, disabled_count);
           });

  // Пробежаться по всем тестам в порядке добавления
  size_t index = 0;

  while (index < tests.size()) {
    // Получить дескриптор теста
    shared_ptr<TestDescriptor> descriptor = tests[index++];

    // Проверить что тест не отключен и вывести об этом информацию
    if (descriptor->IsDisabled()) {
      for_each(outputters.begin(), outputters.end(),
               [&descriptor](Outputter *outputter) {
                 outputter->SkipDisabledTest(
                     descriptor->getModuleName(), descriptor->getTestName(),
                     descriptor->getRuns(), descriptor->getIterations());
               });
      continue;
    }

    // Вывести информацию о начале конкретного теста
    for_each(outputters.begin(), outputters.end(),
             [&descriptor](Outputter *outputter) {
               outputter->BeginTest(
                   descriptor->getModuleName(), descriptor->getTestName(),
                   descriptor->GetType(), descriptor->getRuns(),
                   descriptor->getIterations());
             });

    shared_ptr<TestsResult> testResult = nullptr;

    if (Benchmark::GetType() == descriptor->GetType()) {
      testResult = Benchmark_(descriptor);
    } else if (TestCase::GetType() == descriptor->GetType()) {
      testResult = TestCase_(descriptor);
    }
    // Вывести результат конкретного теста
    for_each(outputters.begin(), outputters.end(),
             [&descriptor, &testResult](Outputter *outputter) {
               outputter->EndTest(descriptor->getModuleName(),
                                  descriptor->getTestName(), *testResult);
             });
    failed_count += testResult->IsError() ? 1 : 0;
  }

  // Вывести информацию о окончании тестирования
  for_each(
      outputters.begin(), outputters.end(),
      [&enabledCount, &failed_count, &disabled_count](Outputter *outputter) {
        outputter->End(enabledCount, failed_count, disabled_count);
      });
}

std::vector<const TestDescriptor *> TestRunner::ListTests() {
  std::vector<const TestDescriptor *> tests;
  TestRunner &instance = Instance();

  std::size_t index = 0;
  while (index < instance.tests_.size()) {
    tests.push_back(instance.tests_[index++].get());
  }
  return tests;
}

void TestRunner::ShuffleTests() {
  TestRunner &instance = Instance();
  std::random_device rdevice;
  std::mt19937 generator(rdevice());
  std::shuffle(instance.tests_.begin(), instance.tests_.end(), generator);
}

std::vector<shared_ptr<TestDescriptor>> TestRunner::GetTests() const {
  std::vector<shared_ptr<TestDescriptor>> tests;

  std::size_t index = 0;
  while (index < tests_.size()) {
    tests.push_back(tests_[index++]);
  }
  return tests;
}

std::shared_ptr<TestsResult>
TestRunner::TestCase_(const shared_ptr<TestDescriptor> &descriptor) {
  string error_description{};
  shared_ptr<TestFactory> factory = descriptor->getFactory().lock();
  // Получить конкретный тест из фабрики.
  auto test(factory->CreateTest());
  // Запустить конкретный тест
  auto [_, error_desc] = test->Run(descriptor->getIterations());
  // Если нашлась ошибка - прервать тестирование
  if (!error_desc.empty()) {
    error_description = error_desc;
  }
  // Вернуть результат одиночного теста
  return make_shared<TestsResult>(descriptor->GetType(), error_description);
}

std::shared_ptr<TestsResult>
TestRunner::Benchmark_(const shared_ptr<TestDescriptor> &descriptor) {
  // Время выполнения одного прохода теста
  vector<uint64_t> runTimes(descriptor->getRuns());
  string error_description{};
  shared_ptr<TestFactory> factory = descriptor->getFactory().lock();
  size_t run = 0;
  //    if (nullptr == factory) {
  while (run < std::max((descriptor->getRuns()), 1UL)) {
    // Получить конкретный тест из фабрики.
    auto test(factory->CreateTest());
    // Запустить конкретный тест
    auto [time, error_desc] = test->Run(descriptor->getIterations());
    // Елси нашлась ошибка - прервать тестирование
    if (!error_desc.empty()) {
      error_description = error_desc;
      break;
    }
    // Сохранить время работы конкретного теста
    runTimes[run] = time;
    ++run;
  }
  //    }
  // Вернуть результат серии повторов
  return make_shared<TestsResult>(descriptor->GetType(), error_description,
                                  runTimes, descriptor->getIterations());
}

/*------------------------------- Test --------------------------------------*/
void Test::Init() {}

void Test::CleanUp() {}

/*----------------------------- TestCase ------------------------------------*/
Test::RunResult TestCase::Run(std::size_t /*iterations*/) {
  // Set up the testing fixture.
  Init();
  uint64_t duration = 0;
  string error_message{};
  try {
    TestBody();
    //    throw std::out_of_range("ups");
  } catch (std::exception &e) {
    error_message = e.what();
  } catch (...) {
    error_message = "Unknown exception caught"s;
  }
  // Tear down the testing fixture.
  CleanUp();
  // Return the duration in nanoseconds.
  return {duration, error_message};
}

/*----------------------------- Benchmark -----------------------------------*/
Test::RunResult Benchmark::Run(std::size_t iterations) {
  std::size_t iteration = iterations;
  // Set up the testing fixture.
  Init();
  uint64_t duration = 0;
  string error_message{};
  try {

    // Get the starting time.
    const auto startTime = std::chrono::steady_clock::now();
    // Run the test body for each iteration.
    while ((iteration--) != 0U) {
      TestBody();
    }
    // Get the ending time.
    const auto endTime = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime -
                                                                    startTime)
                   .count();

  } catch (std::exception &e) {
    error_message = e.what();
  } catch (...) {
    error_message = "Unknown exception caught"s;
  }
  // Tear down the testing fixture.
  CleanUp();
  // Return the duration in nanoseconds.
  return {duration, error_message};
}

/*--------------------------- Outputter --------------------------------------*/
void Outputter::WriteTestNameToStream(std::ostream &stream,
                                      const std::string &moduleName,
                                      const std::string &testName) {
  stream << moduleName << "." << testName;
}

ConsoleOutputter::ConsoleOutputter(std::ostream &stream) : stream_(stream) {}

void ConsoleOutputter::Begin(const std::size_t &enabled_count,
                             const std::size_t &disabled_count) {
  stream_ << std::fixed;
  stream_ << Console::TextGreen << "[==========]" << Console::TextDefault
          << " Running " << enabled_count
          << (enabled_count == 1 ? " test." : " tests");

  if (disabled_count != 0U) {
    stream_ << ", skipping " << disabled_count
            << (disabled_count == 1 ? " test." : " tests");
  } else {
    stream_ << ".";
  }

  stream_ << std::endl;
}

void ConsoleOutputter::End(const std::size_t &executed_count,
                           const std::size_t &failed_count,
                           const std::size_t &disabled_count) {
  stream_ << Console::TextGreen << "[==========]" << Console::TextDefault
          << " Ran " << executed_count
          << (executed_count == 1 ? " test" : " tests");

  if (failed_count != 0U) {
    stream_ << ", failed " << failed_count
            << (failed_count == 1 ? " test" : " tests");
  } else if (disabled_count != 0U) {
    stream_ << ", skipped " << disabled_count
            << (disabled_count == 1 ? " test" : " tests");
  }
  stream_ << "." << std::endl;
}

void ConsoleOutputter::BeginTest(const std::string &module_name,
                                 const std::string &test_name,
                                 const size_t test_type,
                                 const std::size_t &runs_count,
                                 const std::size_t &iterations_count) {
  BeginOrSkipTest(module_name, test_name, test_type, runs_count,
                  iterations_count, false);
}

void ConsoleOutputter::SkipDisabledTest(const std::string &module_name,
                                        const std::string &test_name,
                                        const std::size_t &runs_count,
                                        const std::size_t &iterations_count) {
  BeginOrSkipTest(module_name, test_name, 0, runs_count, iterations_count,
                  true);
}

void ConsoleOutputter::EndTest(const std::string &module_name,
                               const std::string &test_name,
                               const TestsResult &result) {

  if (TestCase::GetType() == result.GetType()) {
    stream_ << "\033[F"sv;
  }
  if (result.IsError()) {
    stream_ << Console::TextRed << "[     FAIL ] "sv << Console::TextYellow;
    WriteTestNameToStream(stream_, module_name, test_name);
    stream_ << ": "sv << result.ErrorMessage() << "("sv
            << result.ErrorMessage().size() << ")"sv << std::endl;
  } else {
    if (Benchmark::GetType() == result.GetType()) {
      stream_ << Console::TextBlue << "[   RUNS   ] "sv << Console::TextDefault
              << "       Average time: "sv
              << std::setprecision(OUTPUT_HALF_PRECISION)
              << (result.RunTimeAverage() / OUTPUT_KILO) << " us "sv
              << std::endl;

      PAD_DEVIATION_INVERSE("Fastest time: "sv,
                            (result.RunTimeMinimum() / (OUTPUT_KILO)),
                            (result.RunTimeAverage() / (OUTPUT_KILO)), "us");
      PAD_DEVIATION_INVERSE("Slowest time: "sv,
                            (result.RunTimeMaximum() / (OUTPUT_KILO)),
                            (result.RunTimeAverage() / (OUTPUT_KILO)), "us"sv);
      stream_ << std::setprecision(OUTPUT_PRECISION);

      PAD(""sv);
      PAD("Average performance: "sv << result.RunsPerSecondAverage()
                                    << " runs/s"sv);

      PAD(""sv);
      stream_ << Console::TextBlue << "[ITERATIONS] "sv << Console::TextDefault
              << std::setprecision(OUTPUT_HALF_PRECISION)
              << "       Average time: "sv
              << result.IterationTimeAverage() / OUTPUT_KILO << " us "sv
              << std::endl;

      PAD_DEVIATION_INVERSE(
          "Fastest time: "sv, (result.IterationTimeMinimum() / (OUTPUT_KILO)),
          (result.IterationTimeAverage() / (OUTPUT_KILO)), "us"sv);
      PAD_DEVIATION_INVERSE(
          "Slowest time: "sv, (result.IterationTimeMaximum() / (OUTPUT_KILO)),
          (result.IterationTimeAverage() / (OUTPUT_KILO)), "us"sv);

      stream_ << std::setprecision(OUTPUT_PRECISION);

      PAD("");
      PAD("Average performance: "sv << result.IterationsPerSecondAverage()
                                    << " iterations/s"sv);
      PAD_DEVIATION("Best performance: "sv,
                    (result.IterationsPerSecondMaximum()),
                    (result.IterationsPerSecondAverage()), "iterations/s"sv);
      PAD_DEVIATION("Worst performance: "sv,
                    (result.IterationsPerSecondMinimum()),
                    (result.IterationsPerSecondAverage()), "iterations/s"sv);
    }

    stream_ << Console::TextGreen << "[     DONE ] "sv << Console::TextYellow;
    WriteTestNameToStream(stream_, module_name, test_name);

    if (TestCase::GetType() == result.GetType()) {
      stream_ << ": result OK"sv << std::endl;
    } else if (Benchmark::GetType() == result.GetType()) {
      stream_ << Console::TextDefault << " ("sv
              << std::setprecision(OUTPUT_PRECISION)
              << (result.TimeTotal() / OUTPUT_MEGA) << " ms)"sv << std::endl;
    }
  }
}

void ConsoleOutputter::BeginOrSkipTest(const std::string &module_name,
                                       const std::string &test_name,
                                       const size_t test_type,
                                       std::size_t runs_count,
                                       std::size_t iterations_count,
                                       const bool skip) {
  if (skip) {
    stream_ << Console::TextCyan << "[ DISABLED ]"sv;
  } else {
    stream_ << Console::TextGreen << "[ RUN      ]"sv;
  }

  stream_ << Console::TextYellow << " "sv;
  WriteTestNameToStream(stream_, module_name, test_name);
  if (Benchmark::GetType() == test_type) {
    stream_ << Console::TextDefault << " ("sv << runs_count
            << (runs_count == 1 ? " run, "sv : " runs, "sv) << iterations_count
            << (iterations_count == 1 ? " iteration per run)"sv
                                      : " iterations per run)"sv);
  }
  stream_ << std::endl;
}

} // namespace test::detail
#undef PAD_DEVIATION_INVERSE
#undef PAD_DEVIATION
#undef PAD
