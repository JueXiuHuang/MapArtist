// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_PATHFINDING_HPP_
#define SRC_PATHFINDING_HPP_

#include <memory>
#include <string>

#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/Game/Vector3.hpp>
#include <botcraft/Game/World/Blockstate.hpp>
#include <botcraft/Game/World/World.hpp>

#include <pf/Vec3.hpp>

#include "./DS/Octree.hpp"

namespace pf = pathfinding;

template <class T1, class T2, class T3, class T4, class T5>
using Finder = pf::MultiGoalFinder<T1, T2, T3, T4, T5>;
using Edge = pf::eval::Octile;
using Estimate = pf::eval::Octile;
using Weight = pf::weight::ConstWeighted<1, 1>;

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
class BotCraftFinder final
    : public TFinder<BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>,
                     pf::Position, TEdge, TEstimate, TWeight> {
 public:
  pf::BlockType getBlockTypeImpl(const pf::Position &pos) const;

  pf::BlockType _getBlockType(const pf::Position &pos) const;

  inline float getFallDamageImpl(
      [[maybe_unused]] const pf::Position &landingPos,
      [[maybe_unused]] const typename pf::Position::value_type &height) const;

  inline bool goImpl(const std::shared_ptr<pf::Path<pf::Position>> &path);

  inline pf::Position getPlayerLocationImpl() const;

  inline int getMinYImpl() const;

  inline int getMaxYImpl() const;

  inline float getBlockExtraCostImpl(const pf::Position &pos) const;

  explicit BotCraftFinder(Botcraft::BehaviourClient *_client,
                          Botcraft::Vector3<int> anchor, bool _use_flash);

  BotCraftFinder &operator=(const BotCraftFinder &other);

  void updateCache(const Botcraft::Vector3<int> &pos);
  std::size_t getCacheNodeCount();

  void setPreferredX(const int &X);
  void enablePreferred(const bool &enable);

 private:
  Botcraft::BehaviourClient *client;
  bool use_flash;

  ds::Octree<pf::BlockType> cache;
  bool enablePref;
  int prefX = 0;
};

using PathFinder = BotCraftFinder<Finder, Edge, Estimate, Weight>;

extern template class BotCraftFinder<Finder, Edge, Estimate, Weight>;

#endif  // SRC_PATHFINDING_HPP_
