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

  virtual inline bool playerMoveImpl(const pf::Position &from,
                                     const pf::Position &to) override
  {
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
        client->GetEntityManager()->GetLocalPlayer();

    pf::Vec3<double> targetPos, realOffset, offset = to - from;
    {
      std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
      pf::Vec3<double> now{local_player->GetPosition().x,
                           local_player->GetPosition().y,
                           local_player->GetPosition().z};

      targetPos = (now.getXZ().floor().offset(0, now.y, 0) + offset)
                      .offset(0.5, 0, 0.5); // stand in the middle of the block
      realOffset = targetPos - now;
      const auto lookAtPos = targetPos.offset(0.0, 1.62, 0.0);
      local_player->LookAt(
          Botcraft::Vector3<double>(lookAtPos.x, lookAtPos.y, lookAtPos.z),
          true);
    }

    const double speed = Botcraft::LocalPlayer::WALKING_SPEED;
    double norm = std::sqrt(realOffset.getXZ().template squaredNorm<double>());

    auto timeElapsed = [](const std::chrono::steady_clock::time_point &a,
                          const std::chrono::steady_clock::time_point &b)
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(b - a)
          .count();
    };

    if (offset.getXZ().abs().sum() > 0)
    {
      if (offset.y >= 0)
      {
        if (offset.y > 0)
        {
          // jump
          {
            std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
            local_player->Jump();
          }

          // wait for jumping higher than the target
          auto startJumpTime = std::chrono::steady_clock::now();
          while (true)
          {
            auto nowTime = std::chrono::steady_clock::now();
            auto untilTime = nowTime + std::chrono::milliseconds(50);
            if (timeElapsed(startJumpTime, nowTime) > 6 * 50)
            {               // 6 ticks
              return false; // jump failed
            }
            else
            {
              std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
              // std::cerr << local_player->GetY() << std::endl << std::flush;
              if (local_player->GetY() >= targetPos.y)
              {
                break;
              }
            }
            Botcraft::Utilities::SleepUntil(untilTime);
          }
        }

        // move x and z
        auto startTime = std::chrono::steady_clock::now(), preTime = startTime;
        Botcraft::Vector3<double> lastPos;
        {
          std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
          lastPos = local_player->GetPosition();
        }
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
              if (std::abs(local_player->GetX() - targetPos.x) +
                      std::abs(local_player->GetZ() - targetPos.z) <
                  1e-2)
              {
                local_player->SetOnGround(false);
                break;
              }
              if (lastPos == local_player->GetPosition())
              {
                local_player->SetX(targetPos.x);
                if (offset.y > 0)
                {
                  local_player->SetY(local_player->GetY() + 0.001);
                }
                else
                {
                  local_player->SetY(targetPos.y);
                }
                local_player->SetZ(targetPos.z);
              }
              local_player->SetPlayerInputsX(targetPos.x -
                                             local_player->GetX() -
                                             local_player->GetSpeedX());
              if (offset.y > 0)
              {
                local_player->SetY(local_player->GetY() + 0.001);
              }
              else
              {
                local_player->SetY(targetPos.y);
              }
              local_player->SetPlayerInputsZ(targetPos.z -
                                             local_player->GetZ() -
                                             local_player->GetSpeedZ());
            }
            else
            {
              local_player->AddPlayerInputsX(delta_v.x);
              if (offset.y > 0)
              {
                local_player->SetY(local_player->GetY() + 0.001);
              }
              else
              {
                local_player->SetY(targetPos.y);
              }
              local_player->AddPlayerInputsZ(delta_v.z);
            }
            lastPos = local_player->GetPosition();
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

        auto startTime = std::chrono::steady_clock::now(), preTime = startTime;
        while (true)
        {
          auto nowTime = std::chrono::steady_clock::now();
          auto untilTime = nowTime + std::chrono::milliseconds(50);
          const double elapsed_t =
              static_cast<double>(timeElapsed(startTime, nowTime));
          const double delta_t =
              static_cast<double>(timeElapsed(preTime, nowTime));
          pf::Vec3<double> delta_v =
              (static_cast<pf::Vec3<double>>(realOffset)) *
              (delta_t / expectTime);
          {
            std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
            if ((elapsed_t / 1000.0) > norm / speed)
            {
              if (std::abs(local_player->GetX() - targetPos.x) +
                      std::abs(local_player->GetZ() - targetPos.z) <
                  1e-2)
              {
                local_player->SetOnGround(false);
                break;
              }
              local_player->SetPlayerInputsX(targetPos.x -
                                             local_player->GetX() -
                                             local_player->GetSpeedX());
              local_player->SetY(local_player->GetY() + 0.001);
              local_player->SetPlayerInputsZ(targetPos.z -
                                             local_player->GetZ() -
                                             local_player->GetSpeedZ());
            }
            else
            {
              local_player->AddPlayerInputsX(delta_v.x);
              local_player->SetY(local_player->GetY() + 0.001);
              local_player->AddPlayerInputsZ(delta_v.z);
            }
          }
          preTime = nowTime;
          Botcraft::Utilities::SleepUntil(untilTime);
        }
      }

      // wait for falling
      double lastY;
      int doNotChangeTime = 0;
      bool lastYInit = false;
      while (offset.y != 0)
      {
        auto nowTime = std::chrono::steady_clock::now();
        auto untilTime = nowTime + std::chrono::milliseconds(50);
        if (local_player->GetOnGround() && local_player->GetSpeedY() == 0)
        {
          break;
        }
        else
        {
          std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
          double nowY = local_player->GetY();
          if (lastYInit && std::abs(nowY - lastY) < 0.003)
          {
            doNotChangeTime++;
            if (doNotChangeTime > 5)
            {
              // player didn't move, something happened
              break;
            }
          }
          else
          {
            lastYInit = true;
            lastY = nowY;
          }
        }
        Botcraft::Utilities::SleepUntil(untilTime);
      }
    }
    else
    {
      const double velocity = (offset.y > 0 ? 0.12 * 0.98 : 0.15);
      double expectTime =
          std::floor(static_cast<double>(std::abs(offset.y)) / velocity) * 50;

      {
        std::lock_guard<std::mutex> player_lock(local_player->GetMutex());
        local_player->SetIsClimbing(true);
        local_player->SetOnGround(false);
      }

      auto startTime = std::chrono::steady_clock::now(), preTime = startTime;
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
            local_player->SetY(targetPos.y);
            local_player->SetZ(targetPos.z);
            break;
          }
          else
          {
            local_player->AddPlayerInputsY(delta_v.y);
          }
        }
        preTime = nowTime;
        Botcraft::Utilities::SleepUntil(untilTime);
      }
    }

    return true;
  }

  BotCraftFinder(std::shared_ptr<Botcraft::BehaviourClient> _client)
      : TFinder<BotCraftFinder<TFinder, TWeight, TEstimate, TEdge>, TWeight, TEstimate, TEdge>(
            {true, 9999999}),
        client(_client) {}

private:
  std::shared_ptr<Botcraft::BehaviourClient> client;
};

#endif