#pragma once

#include <cmath>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

//
// Class for scheduling tasks.
// Tasks are added to the scheduler by add() method.
// Doesn`t contain gain arguments.
// Result of a task can be bound to an argument of another
// task by getFutureResult() method.
// Result of a task can be received by getResult() method.
//
class TTaskScheduler {
  //
  // Contains simple id`s of tasks.
  //
  struct TaskId_ {
    //
    // Its forced use prevents
    // independent use of id by the user
    // which can lead to errors.
    //

    constexpr explicit TaskId_(size_t n) : id_(n) {}

    TaskId_(const TaskId_& another) = default;
    TaskId_(TaskId_&& another) = default;
    TaskId_& operator=(const TaskId_& another) = default;
    TaskId_& operator=(TaskId_&& another) = default;

    size_t getId() const { return id_; }

   private:
    size_t id_;
  };

  //
  // Top type erasure abstraction for Arguments
  //
  template <class RtrnT>
  struct AbstractArgument_ {
    virtual ~AbstractArgument_() = default;
    virtual RtrnT get() = 0;
  };

  //
  // Wrapping class for usual arguments (like float, int...).
  //
  template <class RtrnT>
  struct SimpleArgument_ final : AbstractArgument_<RtrnT> {
    explicit SimpleArgument_(RtrnT arg) : arg_(arg) {}
    RtrnT get() override { return arg_; }

   private:
    RtrnT arg_;
  };

  template <class RtrnT>
  struct BaseTask_;

  //
  // Top level type erasure abstraction for Tasks
  //
  struct AnyTask_ {
    template <typename T>
    [[nodiscard]] T getRes() {
      auto derived = dynamic_cast<BaseTask_<T>*>(this);
      if (!derived)
        throw std::bad_cast();
      return derived->getResImpl();
    }

    virtual void eval() = 0;

    bool getStatus() const { return isEvaulated_; }

   protected:
    bool isEvaulated_ = false;
  };

  //
  // Intermediate type erasure abstraction for Task
  //
  template <class RtrnT>
  struct BaseTask_ : AnyTask_ {
    virtual RtrnT getResImpl() = 0;
  };

  //
  // Contains arguments to function call,
  // function itself and result of it`s invocation.
  //
  template <class FncT, typename... Args>
  struct Task_ final : public BaseTask_<std::invoke_result_t<FncT, Args...>> {
    explicit Task_(FncT fnc, std::shared_ptr<AbstractArgument_<Args>>... args)
        : fnc_(fnc), args_(args...) {}

    ~Task_() = default;

    void eval() override {
      res_ = std::apply([this](auto&&... args) { return fnc_(args->get()...); },
                        args_);
      this->isEvaulated_ = true;
    }

    std::invoke_result_t<FncT, Args...> getResImpl() override { return res_; }

   private:
    FncT fnc_;
    std::tuple<std::shared_ptr<AbstractArgument_<Args>>...> args_;
    std::invoke_result_t<FncT, Args...> res_;
  };

  //
  // Wrapping class for using another task like argument
  //
  template <typename RtrnT>
  struct FutureResult_ final : public AbstractArgument_<RtrnT> {
    explicit FutureResult_(std::shared_ptr<AnyTask_> sm)
        : task_(std::move(sm)) {}

    RtrnT get() override {
      if (!task_->getStatus())
        task_->eval();
      return task_->getRes<RtrnT>();
    }

   private:
    std::shared_ptr<AnyTask_> task_;
  };

  template <typename T>
  struct IsFutureResult : std::false_type {};

  template <typename T>
  struct IsFutureResult<FutureResult_<T>> : std::true_type {};

 public:
  TTaskScheduler() = default;
  TTaskScheduler(const TTaskScheduler& another) = default;
  TTaskScheduler(TTaskScheduler&& another) = default;
  TTaskScheduler& operator=(const TTaskScheduler& another) = default;
  TTaskScheduler& operator=(TTaskScheduler&& another) = default;

  //
  // Adding a task to the scheduler, returns its internal id.
  // Doesn`t checks cycling dependings!
  //
  template <typename FuncT, typename... Args>
  [[nodiscard]] constexpr TaskId_ add(FuncT func, Args&&... args) {
    auto task = std::make_unique<
        Task_<FuncT, GetClearType_t_<GetWrappedType_t_<Args>>...>>(
        func, WrapArgument_(std::forward<Args>(args))...);
    tasks_.emplace_back(std::move(task));
    return TaskId_(tasks_.size() - 1);  // numeration is from 0
  }

  //
  // Executes all tasks which are not executed yet
  //
  void executeAll() {
    for (const auto& task : tasks_)
      if (!task->getStatus())
        task->eval();
  };

  //
  // Intendent to bind the result of a task to an argument of another task.
  //
  template <typename T>
  [[nodiscard]] auto getFutureResult(const TaskId_& id) -> FutureResult_<T> {
    return FutureResult_<T>{tasks_[id.getId()]};
  }

  template <typename T>
  [[nodiscard]] T getResult(const TaskId_& tid) {
    const size_t id = tid.getId();
    if (!tasks_[id]->getStatus())
      tasks_[id]->eval();
    return tasks_[id]->getRes<T>();
  }

 private:
  //
  // FutureResult_<T> -> FutureResult_<T>
  // T -> UsualArg<T>
  //
  template <typename T>
  static auto WrapArgument_(T arg) {
    if constexpr (IsFutureResult<T>::value)
      return std::make_unique<T>(arg);
    else
      return std::make_unique<SimpleArgument_<T>>(arg);  // ... else wrapping
  }

  //
  // Gets type of WrapArgument_(T) call
  //
  template <typename T>
  struct GetWrappedType_ {
    using type = decltype(WrapArgument_(std::forward<T>(std::declval<T>())));
  };

  template <typename T>
  using GetWrappedType_t_ = typename GetWrappedType_<T>::type;

  //
  // AbstractArgument_<T> -> T, unwrapping in other words
  //
  template <typename T>
  struct GetClearType_ {
    using type = decltype(std::declval<T>()->get());
  };

  template <typename T>
  using GetClearType_t_ = typename GetClearType_<T>::type;

  std::vector<std::shared_ptr<AnyTask_>> tasks_;
};