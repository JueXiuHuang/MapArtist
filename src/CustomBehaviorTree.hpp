// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_CUSTOMBEHAVIORTREE_HPP_
#define SRC_CUSTOMBEHAVIORTREE_HPP_

#include <string>

#include <botcraft/AI/BehaviourTree.hpp>

/// @brief A Decorator that ticks its child until
/// it successes. If ticked n times with no
/// success, return failure.
/// @tparam Context The tree context type
template <class Context>
class RepeatUntilSuccess : public Botcraft::Decorator<Context> {
 public:
  RepeatUntilSuccess(const std::string &s, const size_t n_)
      : Botcraft::Decorator<Context>(s), n(n_) {}

 protected:
  Botcraft::Status TickImpl(Context &context) const override {
    for (size_t i = 0; i < n; ++i) {
      Botcraft::Status child_return = this->TickChild(context);
      if (child_return == Botcraft::Status::Success)
        return Botcraft::Status::Success;
    }
    return Botcraft::Status::Failure;
  }

 private:
  size_t n;
};

#endif  // SRC_CUSTOMBEHAVIORTREE_HPP_
