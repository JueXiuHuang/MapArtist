#ifndef PATHFINDING_HPP_
#define PATHFINDING_HPP_

#include <Evaluate/Evaluate.hpp>
#include <Finder/Finder.hpp>
#include <Goal/Goal.hpp>
#include <Weighted/Weighted.hpp>

#include "botcraft/AI/SimpleBehaviourClient.hpp"
#include "botcraft/AI/Tasks/PathfindingTask.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Physics/PhysicsManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "botcraft/Utilities/MiscUtilities.hpp"

namespace Botcraft {
  bool Move(BehaviourClient& client, std::shared_ptr<LocalPlayer>& local_player, const Position& target_pos, const float speed, const float climbing_speed);
}

namespace pf = pathfinding;

template <template <typename, typename, typename, typename> class TFinder = pf::AstarFinder,
          class TWeight = pf::weight::ConstWeighted<1, 1>,
          class TEstimate = pf::eval::Manhattan,
          class TEdge = pf::eval::Manhattan>
class BotCraftFinder final
    : public TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>
{
public:
  virtual std::string getBlockNameImpl(const pf::Position &pos) const override
  {
    // get block information
    auto world = client->GetWorld();
    if (world->IsLoaded(Botcraft::Position{pos.x, pos.y, pos.z}))
    {
      const Botcraft::Blockstate *block =
          world->GetBlock(Botcraft::Position{pos.x, pos.y, pos.z});
      return (block != nullptr ? block->GetName() : "minecraft:air");
    }
    else
    {
      return "";
    }
  }

  virtual std::vector<std::string> getBlockNameImpl(
      const std::vector<pf::Position> &pos) const override
  {
    std::vector<Botcraft::Position> botcraftPos;
    botcraftPos.reserve(pos.size());
    for (const pf::Position &p : pos)
    {
      botcraftPos.emplace_back(p.x, p.y, p.z);
    }

    // get block information
    auto world = client->GetWorld();
    std::vector<const Botcraft::Blockstate *> blocks =
        world->GetBlocks(botcraftPos);

    std::vector<std::string> blockNames;
    for (int i = 0; i < pos.size(); ++i)
    {
      if (world->IsLoaded(botcraftPos[i]))
      {
        blockNames.emplace_back(
            (blocks[i] != nullptr ? blocks[i]->GetName() : "minecraft:air"));
      }
      else
      {
        blockNames.emplace_back("");
      }
    }

    return blockNames;
  }

  virtual std::vector<pf::BlockType> getBlockTypeImpl(
      const std::vector<pf::Position> &pos) const override
  {
    std::vector<Botcraft::Position> botcraftPos;
    botcraftPos.reserve(pos.size());
    for (const pf::Position &p : pos)
    {
      botcraftPos.emplace_back(p.x, p.y, p.z);
    }

    // get block information
    auto world = client->GetWorld();
    std::vector<const Botcraft::Blockstate *> blocks =
        world->GetBlocks(botcraftPos);

    std::vector<pf::BlockType> blockTypes;
    for (int i = 0; i < blocks.size(); ++i)
    {
      if (world->IsLoaded(botcraftPos[i]))
      {
        if (blocks[i] != nullptr)
        {
          if (blocks[i]->IsHazardous())
          {
            blockTypes.emplace_back(pf::BlockType::DANGER, pf::BlockType::NONE);
          }
          else if (blocks[i]->IsAir())
          {
            blockTypes.emplace_back(pf::BlockType::AIR,
                                    pf::BlockType::FORCE_DOWN);
          }
          else if (blocks[i]->IsClimbable())
          {
            blockTypes.emplace_back(pf::BlockType::SAFE,
                                    pf::BlockType::CAN_UP_DOWN);
          }
          else if (blocks[i]->IsTransparent())
          {
            // we don't stand on a not full block (1x1x1)
            // except this
            if (blocks[i]->GetName().find("slab") != std::string::npos ||
                blocks[i]->GetName().find("stairs") != std::string::npos ||
                blocks[i]->GetName().find("carpet") != std::string::npos)
            {
              blockTypes.emplace_back(pf::BlockType::SAFE, pf::BlockType::NONE);
            }
            else if (blocks[i]->IsSolid())
            {
              blockTypes.emplace_back(pf::BlockType::DANGER,
                                      pf::BlockType::NONE);
            }
            else
            {
              blockTypes.emplace_back(pf::BlockType::AIR, pf::BlockType::NONE);
            }
          }
          else if (blocks[i]->IsSolid())
          {
            blockTypes.emplace_back(pf::BlockType::SAFE, pf::BlockType::NONE);
          }
          else
          {
            blockTypes.emplace_back(pf::BlockType::SAFE, pf::BlockType::NONE);
          }
        }
        else
        {
          blockTypes.emplace_back(pf::BlockType::AIR,
                                  pf::BlockType::FORCE_DOWN);
        }
      }
      else
      {
        blockTypes.emplace_back(pf::BlockType::UNKNOWN, pf::BlockType::NONE);
      }
    }
    return blockTypes;
  }

  virtual inline float getFallDamageImpl(
      [[maybe_unused]] const pf::Position &landingPos,
      [[maybe_unused]] const typename pf::Position::value_type &height)
      const override
  {
    return 0.0;
  }

  virtual inline bool goImpl(
      const std::shared_ptr<pf::Path<pf::Position>>& path) override {
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
        client->GetEntityManager()->GetLocalPlayer();
    std::shared_ptr<Botcraft::PhysicsManager> physics_manager =
        client->GetPhysicsManager();

    // always set gravity to true to calculate y correctly
    const bool has_gravity = physics_manager->GetHasGravity();
    physics_manager->SetHasGravity(true);
    // Reset gravity before leaving
    Botcraft::Utilities::OnEndScope reset_gravity([physics_manager, has_gravity]() {
      if (physics_manager != nullptr)
        physics_manager->SetHasGravity(has_gravity);
    });

    const float walk_speed = Botcraft::LocalPlayer::WALKING_SPEED;
    const float climb_speed = 0.12 * 20;

    auto& pathVec = path->get();
    // move player, but skipping first position
    for (int i = 1; i < pathVec.size(); ++i) {
      const pf::Position &prevPos = pathVec[i - 1], &newPos = pathVec[i],
                 diffPos = newPos - prevPos;
      std::cout << "From: " << prevPos << " To: " << newPos
                << " Diff: " << diffPos << " (" << i << "/"
                << (path->size() - 1) << ")" << std::endl
                << std::flush;

      bool succeed =
          Botcraft::Move(*client, local_player,
                         Botcraft::Position(newPos.x, newPos.y + 1, newPos.z),
                         walk_speed, climb_speed);
      if (!succeed) {
        return false;
      }
    }
    return true;
  }

  BotCraftFinder(std::shared_ptr<Botcraft::BehaviourClient> _client)
      : TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>(
            {true, 9999999}),
        client(_client) {}

  BotCraftFinder& operator=(const BotCraftFinder &other){
    this->client = other.client;
    this->config = other.config;
  }

private:
  std::shared_ptr<Botcraft::BehaviourClient> client;
};

#endif