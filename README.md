# TTaskScheduler

Моё решение лабораторной работы по курсу "Программирование на C++"
Задачей было написать ленивый планировщик задач.

По изначальному условию в каждой задаче не более двух аргументов.
Я реализовал без этого ограничения, применив variadic templates и perfect forwarding.

Сборка реализована через CMake.

*Оригинальное ТЗ можно найти в файле Task.md.*

Examples:

```
  TTaskScheduler scheduler;

  // Add tasks
  auto task1 = scheduler.add([](int x) { return x * 2; }, 10);
  auto task2 = scheduler.add([](int a, int b) { return a + b; }, 5,
                             scheduler.getFutureResult<int>(task1));

  // Execute all tasks
  scheduler.executeAll();

  // Retrieve results
  int result1 = scheduler.getResult<int>(task1); // 20
  int result2 = scheduler.getResult<int>(task2); // 25
```

Lazy working:
```
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

  scheduler.executeAll(); // id2 task will be executed only once

  ASSERT_EQ(scheduler.getResult<float>(id5), 2);
  ASSERT_EQ(scheduler.getResult<float>(id6), 0);
```
