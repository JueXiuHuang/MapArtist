#ifndef PATHFINDING_HPP_
#define PATHFINDING_HPP_

#include <Evaluate/Evaluate.hpp>
#include <Finder/Finder.hpp>
#include <Goal/Goal.hpp>
#include <Weighted/Weighted.hpp>

#include "botcraft/AI/SimpleBehaviourClient.hpp"
#include "botcraft/AI/Tasks/PathfindingTask.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "botcraft/Utilities/MiscUtilities.hpp"

namespace Botcraft
{
  bool Move(BehaviourClient &client, std::shared_ptr<LocalPlayer> &local_player, const Position &target_pos, const float speed_factor, const bool sprint);
  void AdjustPosSpeed(BehaviourClient &client);
}

namespace pf = pathfinding;

template <template <class, class, class, class> class TFinder = pf::AstarFinder,
          class TWeight = pf::weight::ConstWeighted<1, 1>,
          class TEstimate = pf::eval::Manhattan,
          class TEdge = pf::eval::Manhattan>
class BotCraftFinder final
    : public TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>
{
public:
  virtual pf::BlockType getBlockTypeImpl(
      const pf::Position &pos) const override
  {
    Botcraft::Position botcraftPos(pos.x, pos.y, pos.z);

    // get block information
    auto world = client->GetWorld();
    const Botcraft::Blockstate *block = world->GetBlock(botcraftPos);

    if (world->IsLoaded(botcraftPos))
    {
      if (block != nullptr)
      {
        if (block->IsHazardous())
        {
          return {pf::BlockType::DANGER, pf::BlockType::NONE};
        }
        else if (block->IsAir())
        {
          return {pf::BlockType::AIR, pf::BlockType::FORCE_DOWN};
        }
        else if (block->IsClimbable())
        {
          return {pf::BlockType::SAFE, pf::BlockType::CAN_UP_DOWN};
        }
        else if (block->IsWallHeight())
        {
          return {pf::BlockType::DANGER, pf::BlockType::NONE};
        }
        else if (block->IsSolid())
        {
          return {pf::BlockType::SAFE, pf::BlockType::NONE};
        }
        else
        {
          return {pf::BlockType::DANGER, pf::BlockType::NONE};
        }
      }
      else
      {
        return {pf::BlockType::AIR, pf::BlockType::FORCE_DOWN};
      }
    }
    else
    {
      return {pf::BlockType::UNKNOWN, pf::BlockType::NONE};
    }
  }

  virtual inline float getFallDamageImpl(
      [[maybe_unused]] const pf::Position &landingPos,
      [[maybe_unused]] const typename pf::Position::value_type &height)
      const override
  {
    return 0.0;
  }

  virtual inline bool goImpl(
      const std::shared_ptr<pf::Path<pf::Position>> &path) override
  {
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
        client->GetEntityManager()->GetLocalPlayer();
    std::shared_ptr<Botcraft::World> world = client->GetWorld();

    const float speed_factor = 1.0;

    auto &pathVec = path->get();
    // move player, but skipping first position
    for (int i = 1; i < pathVec.size(); ++i)
    {
      const pf::Position &prevPos = pathVec[i - 1], &newPos = pathVec[i],
                         diffPos = newPos - prevPos;
      std::cout << "From: " << prevPos << " To: " << newPos
                << " Diff: " << diffPos << " (" << i << "/"
                << (path->size() - 1) << ")" << std::endl
                << std::flush;

      // Wait until we are on the ground or climbing
      const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
      while (!local_player->GetOnGround() && !local_player->IsClimbing() && !local_player->IsInFluid())
      {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 2000)
        {
          LOG_WARNING("Timeout waiting for the bot to land on the floor between two block move. Staying at " << local_player->GetPosition());
          break;
        }
        client->Yield();
      }

      // Basic verification to check we won't try to walk on air.
      // If so, it means some blocks have changed, better to
      // recompute a new path
      const Botcraft::Blockstate *next_target = world->GetBlock(Botcraft::Position(newPos.x, newPos.y, newPos.z));
      const Botcraft::Blockstate *above = world->GetBlock(Botcraft::Position(newPos.x, newPos.y + 1, newPos.z));
      if ((above == nullptr || (!above->IsClimbable() && !above->IsFluid())) &&
          (next_target == nullptr || next_target->IsAir()))
      {
        break;
      }

      // If something went wrong, break and
      // replan the whole path to the goal
      auto Step = [&](){
        return Move(*client, local_player, Botcraft::Position(newPos.x, newPos.y + 1, newPos.z), speed_factor, true);
      };
      if (!(Step() || Step() || Step()))  // 3 chances
      {
        return false;
      }
    }
    AdjustPosSpeed(*client);
    return true;
  }

  virtual inline pf::Position getPlayerLocationImpl() const override
  {
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
        client->GetEntityManager()->GetLocalPlayer();
    auto player_pos = local_player->GetPosition();
    return {static_cast<int>(std::floor(player_pos.x)),
            // Get the position, we add 0.25 to Y in case we are at X.97 instead of X+1
            static_cast<int>(std::floor(player_pos.y + 0.25)) - 1,
            static_cast<int>(std::floor(player_pos.z))};
  }

  virtual inline int getMinYImpl() const override
  {
    return client->GetWorld()->GetMinY();
  }

  virtual inline int getMaxYImpl() const override
  {
    return client->GetWorld()->GetHeight();
  }

  BotCraftFinder(Botcraft::BehaviourClient *_client)
      : TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>(
            {false, 9999999}), // do not use 8-connect
        client(_client)
  {
  }

  BotCraftFinder &operator=(const BotCraftFinder &other)
  {
    this->client = other.client;
    this->config = other.config;
  }

private:
  Botcraft::BehaviourClient *client;
};

using PathFinder = BotCraftFinder<>;

#endif