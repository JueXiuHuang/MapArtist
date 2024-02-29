#ifndef PATHFINDING_HPP_
#define PATHFINDING_HPP_

#include <bitset>
#include <functional>
#include <Evaluate/Evaluate.hpp>
#include <Finder/Finder.hpp>
#include <Goal/Goal.hpp>
#include <Weighted/Weighted.hpp>

#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/Tasks/PathfindingTask.hpp>
#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Utilities/Logger.hpp>
#include <botcraft/Utilities/MiscUtilities.hpp>

// Forward Declarations
std::string GetTime();

namespace Botcraft
{
  Status StopFlying(BehaviourClient& client);
  void AdjustPosSpeed(BehaviourClient& client);

  // a75f87e0-0583-435b-847a-cf0c18ede2d1
  static constexpr std::array<unsigned char, 16> botcraft_pathfinding_speed_uuid = {
      0xA7, 0x5F, 0x87, 0xE0,
      0x05, 0x83,
      0x43, 0x5B,
      0x84, 0x7A,
      0xCF, 0x0C, 0x18, 0xED, 0xE2, 0xD1 };

  static bool Move(BehaviourClient& client, std::shared_ptr<LocalPlayer>& local_player, const Position& target_pos, const float speed_factor, const bool sprint, std::function<bool(void)>& checkTP)
  {
    const Vector3<double> target_position(target_pos.x + 0.5, target_pos.y, target_pos.z + 0.5);
    const Vector3<double> look_at_target = target_position + Vector3<double>(0.0, local_player->GetEyeHeight(), 0.0);
    const Vector3<double> motion_vector = target_position - local_player->GetPosition();
    const double half_player_width = 0.5 * local_player->GetWidth();
    const Vector3<double> horizontal_target_position(target_position.x, 0.0, target_position.z);

    std::shared_ptr<World> world = client.GetWorld();

    if (speed_factor != 1.0f)
    {
      local_player->SetAttributeModifier(
        EntityAttribute::Type::MovementSpeed,
        botcraft_pathfinding_speed_uuid,
        EntityAttribute::Modifier{
            speed_factor - 1.0f, // -1 as MultiplyTotal will multiply by (1.0 + x)
            EntityAttribute::Modifier::Operation::MultiplyTotal });
    }
    Utilities::OnEndScope botcraft_speed_modifier_remover([&]()
      { local_player->RemoveAttributeModifier(EntityAttribute::Type::MovementSpeed, botcraft_pathfinding_speed_uuid); });

    local_player->LookAt(look_at_target, true);

    // Move horizontally (with a jump if necessary)
    if (std::abs(motion_vector.x) > 0.5 || std::abs(motion_vector.z) > 0.5)
    {
      // If we need to jump
      if (motion_vector.y > 0.5 || std::abs(motion_vector.x) > 1.5 || std::abs(motion_vector.z) > 1.5)
      {
        // If not a jump above a gap, wait until reaching the next Y value before moving X/Z
        if (std::abs(motion_vector.x) < 1.5 && std::abs(motion_vector.z) < 1.5)
        {
          local_player->SetInputsJump(true);

          if (!Utilities::YieldForCondition([&]() -> bool
            {
              if (checkTP()) { return true; }
              return local_player->GetY() >= static_cast<float>(target_pos.y) * 0.85;
            }, client, 2000))
          {
            return false;
          }
        }
        // It's a long jump above a gap
        else
        {
          const Vector3<double> current_block_center_xz(
            std::floor(local_player->GetX()) + 0.5,
            0.0,
            std::floor(local_player->GetZ()) + 0.5);
          // Start to move forward to have speed before jumping
          if (!Utilities::YieldForCondition([&]() -> bool
            {
              if (checkTP()) { return true; }
              if (local_player->GetDirtyInputs())
              {
                return false;
              }

              // We need to add the forward input even for the tick we do the jump
              local_player->LookAt(look_at_target, false);
              local_player->SetInputsForward(1.0);
              local_player->SetInputsSprint(sprint);

              if (Vector3<double>(local_player->GetX(), 0.0, local_player->GetZ()).SqrDist(current_block_center_xz) > half_player_width * half_player_width)
              {
                // Then jump
                local_player->SetInputsJump(true);
                return true;
              }

              return false; },
            client, 1000))
          {
            return false;
          }
        }
      }
      // Move forward to the right X/Z location
      if (!Utilities::YieldForCondition([&]() -> bool
        {
          if (checkTP()) { return true; }
          if (local_player->GetDirtyInputs())
          {
            return false;
          }

          const Vector3<double> current_pos = local_player->GetPosition();
          if (Vector3<double>(current_pos.x, 0.0, current_pos.z).SqrDist(horizontal_target_position) < (0.5 - half_player_width) * (0.5 - half_player_width))
          {
            return true;
          }

          local_player->LookAt(look_at_target, false);
          const Vector3<double> speed = local_player->GetSpeed();
          double forward = 1.0;
          // If we need to fall, stop accelerating to prevent "overshooting" and potentially
          // hit some block on the bottom or on the side of the dropshoot
          if (motion_vector.y < -0.5 &&
            static_cast<int>(std::floor(current_pos.x)) == target_pos.x &&
            static_cast<int>(std::floor(current_pos.z)) == target_pos.z)
          {
            if (std::max(std::abs(speed.x), std::abs(speed.z)) > 0.12) // 0.12 because I needed a value so why not
            {
              forward = -1.0;
            }
            else if (std::max(std::abs(speed.x), std::abs(speed.z)) > 0.06)
            {
              forward = 0.0;
            }
          }
          local_player->SetInputsForward(forward);
          local_player->SetInputsSprint(sprint && (forward == 1.0));

          return false; },
        client, (std::abs(motion_vector.x) + std::abs(motion_vector.z) + (motion_vector.y < -0.5)) * 1000))
      {
        return false;
      }
    }

    // If we need to go down, let the gravity do it's job, unless we are in a scaffholding or water,
    // in which case we need to press sneak to go down. If free falling in air, press sneak to catch
    // climbable at the bottom, preventing fall damage
    if (local_player->GetY() > target_position.y && !Utilities::YieldForCondition([&]() -> bool
      {
        if (checkTP()) { return true; }
        // Previous inputs have not been processed by physics thread yet
        if (local_player->GetDirtyInputs())
        {
          return false;
        }

        if (static_cast<int>(std::floor(local_player->GetY())) <= target_pos.y &&
          (local_player->GetOnGround() || local_player->IsClimbing() || local_player->IsInFluid()))
        {
          return true;
        }

        const Vector3<double> current_pos = local_player->GetPosition();
        const Blockstate* feet_block = world->GetBlock(Position(
          static_cast<int>(std::floor(current_pos.x)),
          static_cast<int>(std::floor(current_pos.y)),
          static_cast<int>(std::floor(current_pos.z))
        ));
        local_player->SetInputsSneak(feet_block != nullptr &&
          (feet_block->IsFluidOrWaterlogged() || feet_block->IsScaffolding() || (feet_block->IsAir() && motion_vector.y < -2.5)));
        local_player->SetInputsJump(local_player->GetFlying()); // Stop flying

        // If we drifted too much, adjust toward target X/Z position
        if (Vector3<double>(current_pos.x, 0.0, current_pos.z).SqrDist(horizontal_target_position) > (0.5 - half_player_width) * (0.5 - half_player_width))
        {
          local_player->LookAt(look_at_target, false);
          local_player->SetInputsForward(1.0);
        }

        return false; },
      client, 1000 + 1000 * std::abs(motion_vector.y)))
    {
      return false;
    }

    // We need to go up (either from ground or in a climbable/water)
    if (local_player->GetY() < target_position.y && !Utilities::YieldForCondition([&]() -> bool
      {
        if (checkTP()) { return true; }
        // Previous inputs have not been processed by physics thread yet
        if (local_player->GetDirtyInputs())
        {
          return false;
        }

        const Vector3<double> current_pos = local_player->GetPosition();
        if (static_cast<int>(std::floor(current_pos.y)) >= target_pos.y)
        {
          return true;
        }

        local_player->SetInputsJump(true);

        // If we drifted too much, adjust toward target X/Z position
        if (Vector3<double>(current_pos.x, 0.0, current_pos.z).SqrDist(horizontal_target_position) > (0.5 - half_player_width) * (0.5 - half_player_width))
        {
          local_player->LookAt(look_at_target, false);
          local_player->SetInputsForward(1.0);
        }

        return false; },
      client, 1000 + 1000 * std::abs(motion_vector.y)))
    {
      return false;
    }

    // We are in the target block, make sure we are not a bit too high
    return Utilities::YieldForCondition([&]() -> bool
      {
        if (checkTP()) { return true; }
        if (local_player->GetDirtyInputs())
        {
          return false;
        }

        // One physics tick climbing down is 0.15 at most, so this should never get too low in theory
        if (local_player->GetY() >= target_position.y && local_player->GetY() - target_position.y < 0.2)
        {
          return true;
        }

        const Vector3<double> current_pos = local_player->GetPosition();
        const Blockstate* feet_block = world->GetBlock(Position(
          static_cast<int>(std::floor(current_pos.x)),
          static_cast<int>(std::floor(current_pos.y)),
          static_cast<int>(std::floor(current_pos.z))
        ));
        local_player->SetInputsSneak(feet_block != nullptr &&
          (feet_block->IsFluidOrWaterlogged() || feet_block->IsScaffolding() || (feet_block->IsAir() && motion_vector.y < -2.5)));
        local_player->SetInputsJump(local_player->GetFlying()); // Stop flying

        // If we drifted too much, adjust toward target X/Z position
        if (Vector3<double>(current_pos.x, 0.0, current_pos.z).SqrDist(horizontal_target_position) > (0.5 - half_player_width) * (0.5 - half_player_width))
        {
          local_player->LookAt(look_at_target, false);
          local_player->SetInputsForward(1.0);
        }

        return false; },
      client, 1000);
  }
}

namespace pf = pathfinding;

template <template <class, class, class, class, class> class TFinder = pf::AstarFinder,
  class TEdge = pf::eval::Manhattan,
  class TEstimate = pf::eval::Manhattan,
  class TWeight = pf::weight::ConstWeighted<1, 1>>
  class BotCraftFinder final
  : public TFinder<BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>, pf::Position, TEdge, TEstimate, TWeight>
{
public:
  virtual pf::BlockType getBlockTypeImpl(
    const pf::Position& pos) const override
  {
    Botcraft::Position botcraftPos(pos.x, pos.y, pos.z);

    // get block information
    auto world = client->GetWorld();
    const Botcraft::Blockstate* block = world->GetBlock(botcraftPos);

    if (world->IsLoaded(botcraftPos))
    {
      if (block != nullptr)
      {
        if (block->IsHazardous())
        {
          return { pf::BlockType::DANGER, pf::BlockType::NONE };
        }
        else if (block->IsAir())
        {
          return { pf::BlockType::AIR, pf::BlockType::FORCE_DOWN };
        }
        else if (block->IsClimbable())
        {
          return { pf::BlockType::SAFE, pf::BlockType::CAN_UP_DOWN };
        }
        else if (block->IsWallHeight())
        {
          return { pf::BlockType::DANGER, pf::BlockType::NONE };
        }
        else if (block->IsTransparent() && block->GetHardness() < 0) // minecraft::light
        {
          return { pf::BlockType::DANGER, pf::BlockType::NONE };
        }
        else if (block->IsTransparent() && !block->IsSolid()) // minecraft::torch
        {
          return { pf::BlockType::AIR, pf::BlockType::FORCE_DOWN };
        }
        else if (block->IsTransparent() && block->IsSolid()) // minecraft::slab
        {
          return { pf::BlockType::DANGER, pf::BlockType::NONE };
        }
        else if (block->IsSolid())
        {
          return { pf::BlockType::SAFE, pf::BlockType::NONE };
        }
        else
        {
          return { pf::BlockType::DANGER, pf::BlockType::NONE };
        }
      }
      else
      {
        return { pf::BlockType::AIR, pf::BlockType::FORCE_DOWN };
      }
    }
    else
    {
      return { pf::BlockType::UNKNOWN, pf::BlockType::NONE };
    }
  }

  virtual inline float getFallDamageImpl(
    [[maybe_unused]] const pf::Position& landingPos,
    [[maybe_unused]] const typename pf::Position::value_type& height)
    const override
  {
    return 0.0;
  }

  virtual inline bool goImpl(
    const std::shared_ptr<pf::Path<pf::Position>>& path) override
  {
    std::shared_ptr<Botcraft::LocalPlayer> local_player = client->GetLocalPlayer();
    std::shared_ptr<Botcraft::World> world = client->GetWorld();

    if (StopFlying(*client) == Botcraft::Status::Failure)
    {
      return false;
    }

    // record previous tp count
    const std::size_t prev_tp_id = static_cast<Artist*>(client)->getTPID();
    std::function<bool(void)> isTPOccur = [&]() -> bool
      {
        bool changed = static_cast<Artist*>(client)->getTPID() > prev_tp_id;
        return changed;
      };

    const float speed_factor = 1.0;

    auto& pathVec = path->get();
    // move player, but skipping first position
    for (int i = 1; i < pathVec.size(); ++i)
    {
      const pf::Position& prevPos = pathVec[i - 1], & newPos = pathVec[i],
        diffPos = newPos - prevPos;
      std::cout << "From: " << prevPos << " To: " << newPos
        << " Diff: " << diffPos << " (" << i << "/"
        << (path->size() - 1) << ")" << std::endl
        << std::flush;

      // check flying
      if (local_player->GetFlying())
      {
        std::cout << GetTime() << "Player is flying...";
        std::cout << GetTime() << "Player Abilities: " << std::bitset<8>(local_player->GetAbilitiesFlags()).to_string();
        local_player->SetAbilitiesFlags(
          (~((unsigned char)0x02)) & local_player->GetAbilitiesFlags());
        std::cout << GetTime() << "Player Abilities: " << std::bitset<8>(local_player->GetAbilitiesFlags()).to_string();
      }
      if (isTPOccur())
      {
        return false;
      }

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
      if (isTPOccur())
      {
        return false;
      }

      // Basic verification to check we won't try to walk on air.
      // If so, it means some blocks have changed, better to
      // recompute a new path
      const Botcraft::Blockstate* next_target = world->GetBlock(Botcraft::Position(newPos.x, newPos.y, newPos.z));
      const Botcraft::Blockstate* above = world->GetBlock(Botcraft::Position(newPos.x, newPos.y + 1, newPos.z));
      if ((above == nullptr || (!above->IsClimbable() && !above->IsFluid())) &&
        (next_target == nullptr || next_target->IsAir()))
      {
        return false;
      }

      // If something went wrong, break and
      // replan the whole path to the goal
      auto Step = [&]()
        {
          return Move(*client, local_player, Botcraft::Position(newPos.x, newPos.y + 1, newPos.z), speed_factor, true, isTPOccur);
        };
      if (!(Step() || Step() || Step())) // 3 chances
      {
        return false;
      }
      if (isTPOccur())
      {
        return false;
      }
    }
    AdjustPosSpeed(*client);
    if (isTPOccur())
    {
      return false;
    }
    return true;
  }

  virtual inline pf::Position getPlayerLocationImpl() const override
  {
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
      client->GetEntityManager()->GetLocalPlayer();
    auto player_pos = local_player->GetPosition();
    return { static_cast<int>(std::floor(player_pos.x)),
      // Get the position, we add 0.25 to Y in case we are at X.97 instead of X+1
      static_cast<int>(std::floor(player_pos.y + 0.25)) - 1,
      static_cast<int>(std::floor(player_pos.z)) };
  }

  virtual inline int getMinYImpl() const override
  {
    return client->GetWorld()->GetMinY();
  }

  virtual inline int getMaxYImpl() const override
  {
    return client->GetWorld()->GetHeight();
  }

  BotCraftFinder(Botcraft::BehaviourClient* _client)
    : TFinder<BotCraftFinder<TFinder, TEdge, TEstimate, TWeight>, pf::Position, TEdge, TEstimate, TWeight>(
      { false, false, 9999999, true }), // do not use 8-connect
    client(_client)
  {
  }

  BotCraftFinder& operator=(const BotCraftFinder& other)
  {
    this->client = other.client;
    this->config = other.config;
  }

private:
  Botcraft::BehaviourClient* client;
};

using PathFinder = BotCraftFinder<>;

#endif