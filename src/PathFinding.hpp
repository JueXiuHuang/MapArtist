// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_PATHFINDING_HPP_
#define SRC_PATHFINDING_HPP_

#include <memory>
#include <string>

#include <botcraft/AI/SimpleBehaviourClient.hpp>

#include <pf/Vec3.hpp>

namespace pf = pathfinding;

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
class BotCraftFinder final
    : public TFinder<BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>,
                     pf::Position, TEdge, TEstimate, TWeight> {
 public:
  pf::BlockType getBlockTypeImpl(const pf::Position &pos) const;

  inline float getFallDamageImpl(
      [[maybe_unused]] const pf::Position &landingPos,
      [[maybe_unused]] const typename pf::Position::value_type &height) const;

  inline bool goImpl(const std::shared_ptr<pf::Path<pf::Position>> &path);

  inline pf::Position getPlayerLocationImpl() const;

  inline int getMinYImpl() const;

  inline int getMaxYImpl() const;

  explicit BotCraftFinder(Botcraft::BehaviourClient *_client, bool _use_flash);

  BotCraftFinder &operator=(const BotCraftFinder &other);

 private:
  Botcraft::BehaviourClient *client;
  bool use_flash;
};

using PathFinder =
    BotCraftFinder<pf::MultiGoalFinder, pf::eval::Manhattan,
                   pf::eval::Manhattan, pf::weight::ConstWeighted<1, 1>>;

extern template class BotCraftFinder<pf::MultiGoalFinder, pf::eval::Manhattan,
                                     pf::eval::Manhattan,
                                     pf::weight::ConstWeighted<1, 1>>;

#endif  // SRC_PATHFINDING_HPP_
