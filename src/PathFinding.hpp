// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_PATHFINDING_HPP_
#define SRC_PATHFINDING_HPP_

#include <memory>
#include <string>

#include <botcraft/AI/SimpleBehaviourClient.hpp>

#include <Vec3.hpp>

// Forward Declarations
std::string GetTime();

namespace Botcraft {
Status StopFlying(BehaviourClient &client);
void AdjustPosSpeed(BehaviourClient &client);
bool MyMove(BehaviourClient &client, std::shared_ptr<LocalPlayer> &local_player,
            const Position &target_pos, const float speed_factor,
            const bool sprint, std::function<bool(void)> &checkTP);
}  // namespace Botcraft

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
      [[maybe_unused]] const typename pf::Position::value_type &height)
      const;

  inline bool goImpl(
      const std::shared_ptr<pf::Path<pf::Position>> &path);

  inline pf::Position getPlayerLocationImpl() const;

  inline int getMinYImpl() const;

  inline int getMaxYImpl() const;

  explicit BotCraftFinder(Botcraft::BehaviourClient *_client);

  BotCraftFinder &operator=(const BotCraftFinder &other);

 private:
  Botcraft::BehaviourClient *client;
};

using PathFinder =
    BotCraftFinder<pf::AstarFinder, pf::eval::Manhattan, pf::eval::Manhattan,
                   pf::weight::ConstWeighted<1, 1>>;

#endif  // SRC_PATHFINDING_HPP_
