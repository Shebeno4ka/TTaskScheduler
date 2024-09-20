#include <gtest/gtest.h>

#include "Scheduler.hpp"

TEST(MultipleArgsTest, test1) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add(
      [](float a1, float a2, float a3, float a4) { return a1 + a2 + a3 + a4; },
      1, 1, 1, 1);

  ASSERT_EQ(scheduler.getResult<float>(id1), 4);
}

TEST(MultipleArgsTest, test2) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add(
      [](float a1, float a2, float a3, float a4) { return a1 + a2 + a3 + a4; },
      1, 1, 1, 1);

  auto id2 = scheduler.add([](float a1) { return a1 * a1; },
                           scheduler.getFutureResult<float>(id1));

  ASSERT_EQ(scheduler.getResult<float>(id2), 16);
}

// Check that the many schedulers can work with the same args
TEST(MultipleShedulersTests, ValidUseSameArgsByTwoSchedulers) {
  TTaskScheduler scheduler1, scheduler2;

  int a = 1;
  int b = 2;

  auto id1 = scheduler1.add([](int a, int b) { return a + b; }, a, b);
  auto id2 = scheduler2.add([](int a, int b) { return a + b; }, a, b);

  scheduler1.executeAll();
  scheduler2.executeAll();

  ASSERT_EQ(scheduler1.getResult<int>(id1), 3);
  ASSERT_EQ(scheduler2.getResult<int>(id2), 3);
}

TEST(SampleTest, test) {
  float a = 1;
  float b = -2;
  float c = 0;
  // x^2 - 2x = 0

  TTaskScheduler scheduler;

  auto id1 = scheduler.add([](float a, float c) { return -4 * a * c; }, a, c);

  auto id2 = scheduler.add([](float b, float v) { return b * b + v; }, b,
                           scheduler.getFutureResult<float>(id1));

  auto id3 = scheduler.add([](float b, float d) { return -b + std::sqrt(d); },
                           b, scheduler.getFutureResult<float>(id2));

  auto id4 = scheduler.add([](float b, float d) { return -b - std::sqrt(d); },
                           b, scheduler.getFutureResult<float>(id2));

  auto id5 = scheduler.add([](float a, float v) { return v / (2 * a); }, a,
                           scheduler.getFutureResult<float>(id3));

  auto id6 = scheduler.add([](float a, float v) { return v / (2 * a); }, a,
                           scheduler.getFutureResult<float>(id4));

  scheduler.executeAll();

  ASSERT_EQ(scheduler.getResult<float>(id5), 2);
  ASSERT_EQ(scheduler.getResult<float>(id6), 0);
}

// We do not need to calculate the same thing twice
TEST(UnnecessaryCalculations, test1) {
  size_t discr_count = 0;
  TTaskScheduler scheduler;
  const float a = 1;

  auto id1 = scheduler.add(
      [&](float a1, float a2) {
        ++discr_count;
        return a1 + a2;
      },
      a, a);

  for (size_t i = 0; i < 100; ++i) {
    id1 = scheduler.add([&](float a1, float a2) { return a1 + a2; }, a,
                        scheduler.getFutureResult<float>(id1));
  }

  scheduler.executeAll();

  ASSERT_EQ(discr_count, 1);
}

TEST(ObjectActions, CopyTest) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add([](int a, int b) { return a + b; }, 1, 1);
  auto id2 = scheduler.add([](int a, int b) { return a + b; },
                           scheduler.getFutureResult<int>(id1),
                           scheduler.getFutureResult<int>(id1));

  TTaskScheduler scheduler2 = scheduler;

  scheduler.executeAll();
  scheduler2.executeAll();

  ASSERT_EQ(scheduler.getResult<int>(id2), scheduler2.getResult<int>(id2));
  ASSERT_EQ(scheduler.getResult<int>(id2), 4);
}

TEST(ObjectActions, SelfAssignment) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add([](int a, int b) { return a + b; }, 1, 1);
  auto id2 = scheduler.add([](int a, int b) { return a + b; },
                           scheduler.getFutureResult<int>(id1),
                           scheduler.getFutureResult<int>(id1));

  scheduler.executeAll();

  ASSERT_EQ(scheduler.getResult<int>(id2), 4);
}

TEST(ObjectActions, MoveTest) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add([](int a, int b) { return a + b; }, 1, 1);
  auto id2 = scheduler.add([](int a, int b) { return a + b; },
                           scheduler.getFutureResult<int>(id1),
                           scheduler.getFutureResult<int>(id1));

  TTaskScheduler scheduler2 = std::move(scheduler);

  scheduler2.executeAll();

  ASSERT_EQ(scheduler2.getResult<int>(id2), 4);
}

TEST(EnvironmentValidness, VariablesUnchangedAfterExecutions) {
  TTaskScheduler scheduler;

  int a = 1;
  int b = 1;

  auto _ = scheduler.add([](int a, int b) { return a + b; }, a, b);

  scheduler.executeAll();

  ASSERT_EQ(a, 1);
  ASSERT_EQ(b, 1);
}

TEST(TaskSchedulerTests, VariablesUnchangedAfterExecution) {
  TTaskScheduler scheduler;

  int a = 1;
  int b = 1;

  auto _ = scheduler.add([](int a, int b) { return a + b; }, a, b);

  scheduler.executeAll();

  ASSERT_EQ(a, 1);
  ASSERT_EQ(b, 1);
}

TEST(TaskSchedulerTests, CopyAndMoveSemantics) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add([](int a, int b) { return a + b; }, 1, 1);
  auto id2 = scheduler.add([](int a, int b) { return a + b; },
                           scheduler.getFutureResult<int>(id1),
                           scheduler.getFutureResult<int>(id1));

  // Test copy constructor
  TTaskScheduler scheduler2 = scheduler;
  scheduler.executeAll();
  scheduler2.executeAll();

  ASSERT_EQ(scheduler.getResult<int>(id2), scheduler2.getResult<int>(id2));
  ASSERT_EQ(scheduler.getResult<int>(id2), 4);

  // Test move constructor
  TTaskScheduler scheduler3 = std::move(scheduler);
  scheduler3.executeAll();

  ASSERT_EQ(scheduler3.getResult<int>(id2), 4);
}

TEST(TaskSchedulerTests, SumOfFourFloats) {
  TTaskScheduler scheduler;

  auto id = scheduler.add(
      [](float a1, float a2, float a3, float a4) { return a1 + a2 + a3 + a4; },
      1, 1, 1, 1);

  ASSERT_FLOAT_EQ(scheduler.getResult<float>(id), 4);
}

TEST(TaskSchedulerTests, FutureResultUsage) {
  TTaskScheduler scheduler;

  auto id1 = scheduler.add(
      [](float a1, float a2, float a3, float a4) { return a1 + a2 + a3 + a4; },
      1, 1, 1, 1);

  auto id2 = scheduler.add([](float a1) { return a1 * a1; },
                           scheduler.getFutureResult<float>(id1));

  ASSERT_FLOAT_EQ(scheduler.getResult<float>(id2), 16);
}

// Check that different schedulers can work with the same args
TEST(TaskSchedulerTests, MultipleSchedulersSameArgs) {
  TTaskScheduler scheduler1, scheduler2;

  int a = 1;
  int b = 2;

  auto id1 = scheduler1.add([](int a, int b) { return a + b; }, a, b);
  auto id2 = scheduler2.add([](int a, int b) { return a + b; }, a, b);

  scheduler1.executeAll();
  scheduler2.executeAll();

  ASSERT_EQ(scheduler1.getResult<int>(id1), 3);
  ASSERT_EQ(scheduler2.getResult<int>(id2), 3);
}

TEST(TaskSchedulerTests, ZeroArguments) {
  TTaskScheduler scheduler;

  auto id = scheduler.add([]() { return 42; });

  scheduler.executeAll();

  ASSERT_EQ(scheduler.getResult<int>(id), 42);
}

TEST(TaskSchedulerTests, LargeNumberOfTasks) {
  TTaskScheduler scheduler;

  for (int i = 0; i < 10000; ++i) {
    scheduler.add([=]() { return i; });
  }

  scheduler.executeAll();
}

TEST(TaskSchedulerTests, DifferentArgumentAndReturnTypes) {
  TTaskScheduler scheduler;

  auto id1 =
      scheduler.add([](std::string s) { return s + " World!"; }, "Hello");
  auto id2 = scheduler.add([](int x) { return x * x; }, 5);
  auto id3 = scheduler.add([](double x, double y) { return x + y; }, 3.5, 2.5);

  scheduler.executeAll();

  ASSERT_EQ(scheduler.getResult<std::string>(id1), "Hello World!");
  ASSERT_EQ(scheduler.getResult<int>(id2), 25);
  ASSERT_DOUBLE_EQ(scheduler.getResult<double>(id3), 6.0);
}

TEST(TaskSchedulerTests, TaskThrowsException) {
  TTaskScheduler scheduler;

  scheduler.add([]() {
    throw std::runtime_error("Error!");
    return 5;
  });

  ASSERT_THROW(scheduler.executeAll(), std::runtime_error);  // Expect exception
}

TEST(TaskSchedulerTests, NestedSchedulers) {
  TTaskScheduler outerScheduler;

  TTaskScheduler innerScheduler;
  auto innerId = innerScheduler.add([]() { return 42; });

  auto outerId = outerScheduler.add([&innerScheduler, innerId]() {
    return innerScheduler.getResult<int>(innerId);
  });

  outerScheduler.executeAll();

  ASSERT_EQ(outerScheduler.getResult<int>(outerId), 42);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}