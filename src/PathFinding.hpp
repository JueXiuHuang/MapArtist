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
#include "botcraft/Utilities/SleepUtilities.hpp"

namespace pf = pathfinding;

template <template <typename, typename, typename, typename> class TFinder = pf::AstarFinder,
          class TWeight = pf::weight::ConstWeighted<1, 1>,
          class TEstimate = pf::eval::Manhattan,
          class TEdge = pf::eval::Manhattan>
class BotCraftFinder final
    : public TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>
{
public:
  virtual std::string getBlockNameImpl(const pf::Position& pos) const override {
    // get block information
    auto world = client->GetWorld();
    if (world->IsLoaded(Botcraft::Position{pos.x, pos.y, pos.z})) {
      const Botcraft::Blockstate* block =
          world->GetBlock(Botcraft::Position{pos.x, pos.y, pos.z});
      return (block != nullptr ? block->GetName() : "minecraft:air");
    } else {
      return "";
    }
  }

  virtual std::vector<std::string> getBlockNameImpl(
      const std::vector<pf::Position>& pos) const override {
    std::vector<Botcraft::Position> botcraftPos;
    botcraftPos.reserve(pos.size());
    for (const pf::Position& p : pos) {
      botcraftPos.emplace_back(p.x, p.y, p.z);
    }

    // get block information
    auto world = client->GetWorld();
    std::vector<const Botcraft::Blockstate*> blocks =
        world->GetBlocks(botcraftPos);

    std::vector<std::string> blockNames;
    for (int i = 0; i < pos.size(); ++i) {
      if (world->IsLoaded(botcraftPos[i])) {
        blockNames.emplace_back(
            (blocks[i] != nullptr ? blocks[i]->GetName() : "minecraft:air"));
      } else {
        blockNames.emplace_back("");
      }
    }

    return blockNames;
  }

  virtual std::vector<pf::BlockType> getBlockTypeImpl(
      const std::vector<pf::Position>& pos) const override {
    std::vector<Botcraft::Position> botcraftPos;
    botcraftPos.reserve(pos.size());
    for (const pf::Position& p : pos) {
      botcraftPos.emplace_back(p.x, p.y, p.z);
    }

    // get block information
    auto world = client->GetWorld();
    std::vector<const Botcraft::Blockstate*> blocks =
        world->GetBlocks(botcraftPos);

    std::vector<pf::BlockType> blockTypes;
    for (int i = 0; i < blocks.size(); ++i) {
      if (world->IsLoaded(botcraftPos[i])) {
        if (blocks[i] != nullptr) {
          if (blocks[i]->IsHazardous()) {
            blockTypes.emplace_back(pf::BlockType::DANGER, pf::BlockType::NONE);
          } else if (blocks[i]->IsSolid()) {
            blockTypes.emplace_back(pf::BlockType::SAFE, pf::BlockType::NONE);
          } else if (blocks[i]->IsAir()) {
            blockTypes.emplace_back(pf::BlockType::AIR,
                                    pf::BlockType::FORCE_DOWN);
          } else {
            blockTypes.emplace_back(pf::BlockType::SAFE, pf::BlockType::NONE);
          }
        } else {
          blockTypes.emplace_back(pf::BlockType::AIR,
                                  pf::BlockType::FORCE_DOWN);
        }
      } else {
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

  virtual inline bool playerMoveImpl(const pf::Position &offset) override
  {
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
        client->GetEntityManager()->GetLocalPlayer();

    pf::Vec3<double> targetPos, realOffset;
    {
      std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
      pf::Vec3<double> now{local_player->GetPosition().x,
                           local_player->GetPosition().y,
                           local_player->GetPosition().z};
      targetPos = (now.floor() + offset)
                      .offset(0.5, 0, 0.5); // stand in the middle of the block
      realOffset = targetPos - now;
      const auto lookAtPos = targetPos.offset(0.0, 1.62, 0.0);
      local_player->LookAt(
          Botcraft::Vector3<double>(lookAtPos.x, lookAtPos.y, lookAtPos.z),
          true);
    }

    double norm = std::sqrt(realOffset.getXZ().template squaredNorm<double>());
    auto startTime = std::chrono::steady_clock::now(), preTime = startTime;

    auto timeElapsed = [](const std::chrono::steady_clock::time_point &a,
                          const std::chrono::steady_clock::time_point &b)
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(b - a)
          .count();
    };

    pf::Vec3<double> realTargetPos = targetPos;
    if (offset.y == 0)
    {
      const double speed = Botcraft::LocalPlayer::WALKING_SPEED;

      while (true)
      {
        auto nowTime = std::chrono::steady_clock::now();
        auto untilTime = nowTime + std::chrono::milliseconds(50);
        const double elapsed_t =
            static_cast<double>(timeElapsed(startTime, nowTime));
        const double delta_t =
            static_cast<double>(timeElapsed(preTime, nowTime));
        pf::Vec3<double> delta_v =
            (static_cast<pf::Vec3<double>>(realOffset) / norm) *
            ((delta_t / 1000.0) * speed);
        {
          std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
          if ((elapsed_t / 1000.0) > norm / speed)
          {
            local_player->SetX(targetPos.x);
            local_player->SetZ(targetPos.z);
            break;
          }
          else
          {
            local_player->AddPlayerInputsX(delta_v.x);
            local_player->AddPlayerInputsZ(delta_v.z);
          }
        }
        preTime = nowTime;
        Botcraft::Utilities::SleepUntil(untilTime);
      }
    }
    else if (offset.y > 0)
    {
      double expectTime;
      auto targetBlock = client->GetWorld()->GetBlock(
          Botcraft::Position{static_cast<int>(std::floor(targetPos.x)),
                             static_cast<int>(std::floor(targetPos.y) - 1),
                             static_cast<int>(std::floor(targetPos.z))});
      if (targetBlock != nullptr && targetBlock->GetName().find("slab") != std::string::npos)
      {
        if (targetBlock->GetVariableValue("type") == "top")
        {
          expectTime = 7 * 0.05 * 1000; // 7 ticks
        }
        else
        {
          expectTime = 9 * 0.05 * 1000; // 9 ticks
          realTargetPos.adjust(0, 0.5, 0);
        }
      }
      else if (targetBlock != nullptr && targetBlock->GetName().find("carpet") != std::string::npos)
      {
        expectTime = 11 * 0.05 * 1000; // 11 ticks
        realTargetPos.adjust(0, 1.0 / 32, 0);
      }
      else
      {
        expectTime = 7 * 0.05 * 1000; // 7 ticks
      }

      // jump
      {
        std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
        local_player->Jump();
      }

      while (true)
      {
        auto nowTime = std::chrono::steady_clock::now();
        auto untilTime = nowTime + std::chrono::milliseconds(50);
        const double elapsed_t =
            static_cast<double>(timeElapsed(startTime, nowTime));
        const double delta_t =
            static_cast<double>(timeElapsed(preTime, nowTime));
        pf::Vec3<double> delta_v = (static_cast<pf::Vec3<double>>(realOffset)) *
                                   (delta_t / expectTime);
        {
          std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
          if (elapsed_t > expectTime)
          {
            local_player->SetX(targetPos.x);
            local_player->SetZ(targetPos.z);
            break;
          }
          else
          {
            local_player->AddPlayerInputsX(delta_v.x);
            local_player->AddPlayerInputsZ(delta_v.z);
          }
        }
        preTime = nowTime;
        Botcraft::Utilities::SleepUntil(untilTime);
      }
    }
    else if (offset.y < 0)
    {
      double expectTime = 0;
      double fallingY = offset.abs().y;
      double velocity = 0;
      while (fallingY > 0)
      {
        velocity = (velocity + 0.08) * 0.98;
        fallingY -= velocity;
        expectTime += 50; // 50 ms, 1 tick
      }
      expectTime -= 50;

      while (true)
      {
        auto nowTime = std::chrono::steady_clock::now();
        auto untilTime = nowTime + std::chrono::milliseconds(50);
        const double elapsed_t =
            static_cast<double>(timeElapsed(startTime, nowTime));
        const double delta_t =
            static_cast<double>(timeElapsed(preTime, nowTime));
        pf::Vec3<double> delta_v = (static_cast<pf::Vec3<double>>(realOffset)) *
                                   (delta_t / expectTime);
        {
          std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
          if (elapsed_t > expectTime)
          {
            local_player->SetX(targetPos.x);
            local_player->SetZ(targetPos.z);
            break;
          }
          else
          {
            local_player->AddPlayerInputsX(delta_v.x);
            local_player->AddPlayerInputsZ(delta_v.z);
          }
        }
        preTime = nowTime;
        Botcraft::Utilities::SleepUntil(untilTime);
      }
    }

    // wait for falling
    while (!local_player->GetOnGround() || local_player->GetSpeedY() > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    {
      std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
      local_player->SetY(realTargetPos.y);
    }

    return true;
  }

  BotCraftFinder(std::shared_ptr<Botcraft::BehaviourClient> _client)
      : TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>(
          {false, 9999999}), client(_client) {}

private:
  std::shared_ptr<Botcraft::BehaviourClient> client;
};