// Copyright 2024 JueXiuHuang, ldslds449

#include "./PathFinding.hpp"  // NOLINT

#include <algorithm>
#include <bitset>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/Tasks/PathfindingTask.hpp>
#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Utilities/MiscUtilities.hpp>

#include <pf/Evaluate/Evaluate.hpp>
#include <pf/Finder/Finder.hpp>
#include <pf/Goal/Goal.hpp>
#include <pf/Weighted/Weighted.hpp>

#include "./Artist.hpp"
#include "./Utils.hpp"

namespace Botcraft {

// forward declaration
Status StopFlying(BehaviourClient &client);
void AdjustPosSpeed(BehaviourClient &client);
bool Move(BehaviourClient &client, std::shared_ptr<LocalPlayer> &local_player,
          const Vector3<double> &target_position, const float speed_factor,
          const bool sprint);
}  // namespace Botcraft

template <class T1, class T2>
Botcraft::Vector3<T1> convert(pf::Vec3<T2> vec) {
  return Botcraft::Vector3<T1>(static_cast<T1>(vec.x), static_cast<T1>(vec.y),
                               static_cast<T1>(vec.z));
}

template <class T1, class T2>
pf::Vec3<T1> convert(Botcraft::Vector3<T2> vec) {
  return pf::Vec3<T1>(static_cast<T1>(vec.x), static_cast<T1>(vec.y),
                      static_cast<T1>(vec.z));
}

bool Flash(Botcraft::BehaviourClient &client,
           std::shared_ptr<Botcraft::LocalPlayer> &local_player,
           const Botcraft::Vector3<double> &target_position) {
  Botcraft::Vector3<double> now_pos = local_player->GetPosition();
  const double flash_threshold = 80.0;
  const uint64_t delay = 50;
  bool final_step = false;
  // loop
  while (!final_step) {
    // determine the target
    Botcraft::Vector3<double> now_target = target_position;
    const Botcraft::Vector3<double> vector = (now_target - now_pos);
    const double norm = vector.SqrNorm();
    if (norm > flash_threshold) {
      const Botcraft::Vector3<double> unit = vector / (std::sqrt(norm));
      now_target = unit * flash_threshold + now_pos;
      final_step = false;
    } else {
      final_step = true;
    }
    // set view
    const Botcraft::Vector3<double> look_at_target =
        target_position +
        Botcraft::Vector3<double>(0.0, local_player->GetEyeHeight(), 0.0);
    local_player->LookAt(look_at_target, true);
    // flash
    local_player->SetPosition(now_target);
    // wait for flash
    Botcraft::Utilities::SleepFor(std::chrono::milliseconds(delay));
    if (!Botcraft::Utilities::YieldForCondition(
            [&]() -> bool {
              return std::sqrt((local_player->GetPosition() - target_position)
                                   .SqrNorm()) < 0.1;
            },
            client, 2000)) {
      return false;
    }
    // break the loop
    if (final_step) break;
  }

  return true;
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
pf::BlockType
BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::getBlockTypeImpl(
    const pf::Position &pos) const {
  Botcraft::Position botcraftPos = convert<int, int>(pos);

  auto world = client->GetWorld();
  if (world->IsLoaded(botcraftPos)) {
    return _getBlockType(pos);
  } else if (cache.in({pos.x, pos.y, pos.z})) {
    return cache.get({pos.x, pos.y, pos.z});
  } else {
    return {pf::BlockType::UNKNOWN, pf::BlockType::NONE};
  }
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
pf::BlockType BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::_getBlockType(
    const pf::Position &pos) const {
  Botcraft::Position botcraftPos = convert<int, int>(pos);

  // get block information
  auto world = client->GetWorld();
  const Botcraft::Blockstate *block = world->GetBlock(botcraftPos);

  if (world->IsLoaded(botcraftPos)) {
    if (block != nullptr) {
      if (block->IsHazardous()) {
        return {pf::BlockType::DANGER, pf::BlockType::NONE};
      } else if (block->IsAir()) {
        return {pf::BlockType::AIR, pf::BlockType::FORCE_DOWN};
      } else if (block->IsClimbable()) {
        return {pf::BlockType::SAFE, pf::BlockType::CAN_UP_DOWN};
      } else if (block->IsWallHeight()) {
        return {pf::BlockType::DANGER, pf::BlockType::NONE};
      } else if (block->IsTransparent() &&
                 block->GetHardness() < 0) {  // minecraft::light
        return {pf::BlockType::AIR, pf::BlockType::FORCE_DOWN};
      } else if (block->IsTransparent() &&
                 !block->IsSolid()) {  // minecraft::torch
        return {pf::BlockType::AIR, pf::BlockType::FORCE_DOWN};
      } else if (block->IsTransparent() && block->IsSolid() &&
                 block->GetHardness() >= 1) {  // minecraft::slab
        return {pf::BlockType::DANGER, pf::BlockType::NONE};
      } else if (block->IsSolid()) {
        // check the block height
        const auto &colliders = block->GetCollidersAtPos(botcraftPos);
        float height = static_cast<float>(pos.y);
        for (const auto &c : colliders) {
          height = std::max(static_cast<float>(c.GetMax().y), height);
        }
        if ((height - pos.y) < 1.0) {
          return {pf::BlockType::DANGER, pf::BlockType::NONE};
        } else {
          return {pf::BlockType::SAFE, pf::BlockType::NONE};
        }
      } else {
        return {pf::BlockType::DANGER, pf::BlockType::NONE};
      }
    } else {
      return {pf::BlockType::AIR, pf::BlockType::FORCE_DOWN};
    }
  } else {
    return {pf::BlockType::UNKNOWN, pf::BlockType::NONE};
  }
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
float BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::getFallDamageImpl(
    [[maybe_unused]] const pf::Position &landingPos,
    [[maybe_unused]] const typename pf::Position::value_type &height) const {
  return static_cast<float>(height <= 4 ? 0.0 : 999999.0);
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
bool BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::goImpl(
    const std::shared_ptr<pf::Path<pf::Position>> &path) {
  std::shared_ptr<Botcraft::LocalPlayer> local_player =
      client->GetLocalPlayer();
  std::shared_ptr<Botcraft::World> world = client->GetWorld();

  if (StopFlying(*client) == Botcraft::Status::Failure) {
    return false;
  }

  // record previous tp count
  const std::size_t prev_tp_id = static_cast<Artist *>(client)->getTPID();
  std::function<bool(void)> isTPOccur = [&]() -> bool {
    bool changed = static_cast<Artist *>(client)->getTPID() > prev_tp_id;
    return changed;
  };

  const float speed_factor = 1.0;

  auto &pathVec = path->get();
  // move player, but skipping first position
  for (int i = 1; i < pathVec.size(); ++i) {
    const pf::Position &prevPos = pathVec[i - 1], &newPos = pathVec[i],
                       diffPos = newPos - prevPos;
    std::cout << "From: " << prevPos << " To: " << newPos
              << " Diff: " << diffPos << " (" << i << "/" << (path->size() - 1)
              << ")" << std::endl
              << std::flush;

    // check flying
    if (local_player->GetFlying()) {
      std::cout << GetTime() << "Player is flying...";
      std::cout
          << GetTime() << "Player Abilities: "
          << std::bitset<8>(local_player->GetAbilitiesFlags()).to_string();
      local_player->SetAbilitiesFlags((~((unsigned char)0x02)) &
                                      local_player->GetAbilitiesFlags());
      std::cout
          << GetTime() << "Player Abilities: "
          << std::bitset<8>(local_player->GetAbilitiesFlags()).to_string();
    }
    if (isTPOccur()) {
      return false;
    }

    // Wait until we are on the ground or climbing
    const std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();
    while (!local_player->GetOnGround() && !local_player->IsClimbing() &&
           !local_player->IsInFluid()) {
      if (std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - start)
              .count() >= 2000) {
        LOG_WARNING(
            "Timeout waiting for the bot to land on the floor between two "
            "block move. Staying at "
            << local_player->GetPosition());
        break;
      }
      client->Yield();
    }
    if (isTPOccur()) {
      return false;
    }

    // Basic verification to check we won't try to walk on air.
    // If so, it means some blocks have changed, better to
    // recompute a new path
    const Botcraft::Blockstate *next_target =
        world->GetBlock(convert<int, int>(newPos));
    const Botcraft::Blockstate *above =
        world->GetBlock(convert<int, int>(newPos.offsetY(1)));
    if ((above == nullptr || (!above->IsClimbable() && !above->IsFluid())) &&
        (next_target == nullptr || next_target->IsAir())) {
      return false;
    }

    // If something went wrong, break and
    // replan the whole path to the goal
    auto Step = [&]() {
      // adding 0.5 and 1.0 to change pf coordinate to
      // botcraft coordinate
      auto target = convert<double, double>(
          static_cast<pf::Vec3<double>>(newPos).offset(0.5, 1, 0.5));
      bool result =
          (use_flash ? Flash(*client, local_player, target)
                     : Move(*client, local_player, target, speed_factor, true));

      if (result == false) {
        std::cerr << "Move Error" << std::endl;
        return result;
      }
      // check the position
      if (getPlayerLocation() != newPos) {
        std::cerr << "Position Error" << std::endl;
        return false;
      }
      return true;
    };
    if (!(Step() || Step() || Step())) {  // 3 chances
      return false;
    }
    if (isTPOccur()) {
      return false;
    }
  }
  AdjustPosSpeed(*client);
  if (isTPOccur()) {
    return false;
  }
  return true;
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
pf::Position BotCraftFinder<TFinder, TEdge, TEstimate,
                            TWeight>::getPlayerLocationImpl() const {
  std::shared_ptr<Botcraft::LocalPlayer> local_player =
      client->GetEntityManager()->GetLocalPlayer();
  auto player_pos = local_player->GetPosition();
  return {static_cast<int>(std::floor(player_pos.x)),
          // Get the position, we add 0.25 to Y in case we are at X.97 instead
          // of X+1
          static_cast<int>(std::floor(player_pos.y + 0.25)) - 1,
          static_cast<int>(std::floor(player_pos.z))};
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
int BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::getMinYImpl() const {
  return client->GetWorld()->GetMinY();
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
int BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::getMaxYImpl() const {
  return client->GetWorld()->GetHeight();
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
void BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::updateCache(
    const Botcraft::Vector3<int> &pos) {
  cache.update({pos.x, pos.y, pos.z}, _getBlockType(convert<int, int>(pos)));
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::BotCraftFinder(
    Botcraft::BehaviourClient *_client, Botcraft::Vector3<int> anchor,
    bool _use_flash)
    : TFinder<BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>, pf::Position,
              TEdge, TEstimate, TWeight>(
          pf::FinderConfig{true, false, true, true}),
      cache({anchor.x, anchor.y, anchor.z},
            {anchor.x + 128, anchor.y + 128, anchor.z + 128},
            pf::BlockType{pf::BlockType::UNKNOWN, pf::BlockType::NONE}) {
  // do not use 8-connect
  client = _client;
  use_flash = _use_flash;
}

template <template <class, class, class, class, class> class TFinder,
          class TEdge, class TEstimate, class TWeight>
BotCraftFinder<TFinder, TEdge, TEstimate, TWeight> &
BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>::operator=(
    const BotCraftFinder<TFinder, TEdge, TEstimate, TWeight> &other) {
  this->client = other.client;
  this->config = other.config;
  return *this;
}

template class BotCraftFinder<pf::MultiGoalFinder, pf::eval::Manhattan,
                              pf::eval::Manhattan,
                              pf::weight::ConstWeighted<1, 1>>;
