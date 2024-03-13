// Copyright 2024 JueXiuHuang, ldslds449

#include "./CustomTask.hpp"  // NOLINT

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <botcraft/Game/AssetsManager.hpp>
#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/Inventory/InventoryManager.hpp>
#include <botcraft/Game/Inventory/Window.hpp>
#include <botcraft/Game/Vector3.hpp>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Network/NetworkManager.hpp>
#include <botcraft/Utilities/MiscUtilities.hpp>
#include <botcraft/Utilities/NBTUtilities.hpp>
#include <botcraft/Utilities/SleepUtilities.hpp>

#include "./Algorithm.hpp"
#include "./Artist.hpp"
#include "./Constants.hpp"
#include "./Discord.hpp"
#include "./PathFinding.hpp"
#include "./Utils.hpp"

Botcraft::Status WaitServerLoad(Botcraft::BehaviourClient &c) {
  std::shared_ptr<Botcraft::LocalPlayer> local_player =
      c.GetEntityManager()->GetLocalPlayer();
  Botcraft::Utilities::WaitForCondition(
      [&]() { return local_player->GetPosition().y < 1000; }, 10000);

  return Botcraft::Status::Success;
}

Botcraft::Status checkInventoryAllClear(Botcraft::BehaviourClient &c) {
  std::shared_ptr<Botcraft::InventoryManager> inventory_manager =
      c.GetInventoryManager();
  std::shared_ptr<Botcraft::Window> playerInv =
      inventory_manager->GetPlayerInventory();
  for (int16_t i = Botcraft::Window::INVENTORY_STORAGE_START;
       i < Botcraft::Window::INVENTORY_HOTBAR_START + 5; i++) {
    const ProtocolCraft::Slot &slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) continue;
    Botcraft::ToolType toolType = Botcraft::AssetsManager::getInstance()
                                      .Items()
                                      .at(slot.GetItemID())
                                      ->GetToolType();
    if (toolType == Botcraft::ToolType::Pickaxe) {
      continue;
    }
    if (toolType == Botcraft::ToolType::Axe) {
      continue;
    }
    if (toolType == Botcraft::ToolType::Shovel) {
      continue;
    }
    if (toolType == Botcraft::ToolType::Shears) {
      continue;
    }

    // Something else in player's inventory
    return Botcraft::Status::Failure;
  }

  // Player's inventory all clear
  return Botcraft::Status::Success;
}

Botcraft::Status CheckArtistBlackboardBoolData(Botcraft::BehaviourClient &c,
                                               const std::string &key) {
  Artist &artist = static_cast<Artist &>(c);

  bool result = artist.board.Get<bool>(key, false);
  std::cout << GetTime() << "Check blackboard bool: " << key
            << ", result: " << result << std::endl;

  return result ? Botcraft::Status::Success : Botcraft::Status::Failure;
}

Botcraft::Status GetFood(Botcraft::BehaviourClient &c,
                         const std::string &food_name) {
  std::shared_ptr<Botcraft::InventoryManager> inventory_manager =
      c.GetInventoryManager();
  Artist &artist = static_cast<Artist &>(c);

  // Sort the chest and make sure the first slot in hotbar is empty
  SortChestWithDesirePlace(c);

  // Food will place in this slot
  SwapItemsInContainer(c, Botcraft::Window::PLAYER_INVENTORY_INDEX,
                       Botcraft::Window::INVENTORY_HOTBAR_START,
                       Botcraft::Window::INVENTORY_OFFHAND_INDEX);

  const std::vector<Botcraft::Position> &chests = artist.conf.chests[food_name];

  std::vector<std::size_t> chests_indices(chests.size());
  for (std::size_t i = 0; i < chests.size(); ++i) {
    chests_indices[i] = i;
  }

  int16_t container_id;
  bool item_taken = false;

  for (std::size_t index = 0; index < chests.size(); ++index) {
    const std::size_t i = chests_indices[index];
    // If we can't open this chest for a reason
    FindPathAndMove(c, chests[i], 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0);
    if (OpenContainer(c, chests[i]) == Botcraft::Status::Failure) continue;

    int16_t player_dst = -1;
    while (true) {
      std::vector<int16_t> slots_src;
      container_id = inventory_manager->GetFirstOpenedWindowId();
      if (container_id == -1) continue;

      const std::shared_ptr<Botcraft::Window> container =
          inventory_manager->GetWindow(container_id);

      const int16_t playerFirstSlot = container->GetFirstPlayerInventorySlot();
      player_dst = playerFirstSlot + 9 * 3;

      const std::map<int16_t, ProtocolCraft::Slot> &slots =
          container->GetSlots();

      slots_src.reserve(slots.size());

      for (auto it = slots.begin(); it != slots.end(); ++it) {
        // Chest is src
        if (it->first >= 0 && it->first < playerFirstSlot &&
            !it->second.IsEmptySlot() &&
            Botcraft::AssetsManager::getInstance()
                    .Items()
                    .at(it->second.GetItemID())
                    ->GetName() == food_name) {
          slots_src.push_back(it->first);
        } else if (it->first >= playerFirstSlot && it->second.IsEmptySlot()) {
          player_dst = it->first;
          break;
        }
      }

      if (slots_src.size() > 0) {
        std::cout << GetTime() << "Found food in chest." << std::endl;
        const int src_index = 0;
        // Try to swap the items
        if (SwapItemsInContainer(c, container_id, slots_src[src_index],
                                 player_dst) == Botcraft::Status::Success) {
          std::cout << GetTime() << "Get food success." << std::endl;
          item_taken = true;
          break;
        }
      } else {  // This means the chest doesn't have any food
        break;
      }
    }

    CloseContainer(c, container_id);

    if (!item_taken) continue;

    // No need to continue loooking in the other chests
    break;
  }

  return item_taken ? Botcraft::Status::Success : Botcraft::Status::Failure;
}

struct SlotWithID {
  ProtocolCraft::Slot slot;
  int id;
};

Botcraft::Status SortChestWithDesirePlace(Botcraft::BehaviourClient &c) {
  SortInventory(c);
  std::shared_ptr<Botcraft::InventoryManager> inventory_manager =
      c.GetInventoryManager();
  std::shared_ptr<Botcraft::Window> playerInv =
      inventory_manager->GetPlayerInventory();
  std::vector<int16_t> taskSrc, taskDst;

  std::vector<Botcraft::ToolType> toolTypes{
      Botcraft::ToolType::Pickaxe, Botcraft::ToolType::Axe,
      Botcraft::ToolType::Shears, Botcraft::ToolType::Shovel};

  for (int i = 0; i < toolTypes.size(); i++) {
    std::vector<SlotWithID> tools;
    for (int j = Botcraft::Window::INVENTORY_STORAGE_START;
         j < Botcraft::Window::INVENTORY_HOTBAR_START + 9; j++) {
      ProtocolCraft::Slot &slot = playerInv->GetSlot(j);
      if (slot.IsEmptySlot()) continue;
      if (Botcraft::AssetsManager::getInstance()
              .Items()
              .at(slot.GetItemID())
              ->GetToolType() == toolTypes[i]) {
        tools.push_back({slot, j});
      }
    }

    auto compareFunc = [](const SlotWithID &a, const SlotWithID &b) {
      auto &itemA =
          Botcraft::AssetsManager::getInstance().Items().at(a.slot.GetItemID());
      auto &itemB =
          Botcraft::AssetsManager::getInstance().Items().at(b.slot.GetItemID());

      if (itemA->GetMaxDurability() != itemB->GetMaxDurability()) {
        return itemA->GetMaxDurability() > itemB->GetMaxDurability();
      } else {
        return Botcraft::Utilities::GetDamageCount(a.slot.GetNBT()) <
               Botcraft::Utilities::GetDamageCount(b.slot.GetNBT());
      }
    };

    if (tools.size() < 1) continue;
    std::sort(tools.begin(), tools.end(), compareFunc);
    switch (toolTypes[i]) {
      case Botcraft::ToolType::Pickaxe:
        // put pickaxe at slot 44
        taskSrc.push_back(tools[0].id);
        taskDst.push_back(Botcraft::Window::INVENTORY_HOTBAR_START + 8);
        break;
      case Botcraft::ToolType::Axe:
        // put axe at slot 43
        taskSrc.push_back(tools[0].id);
        taskDst.push_back(Botcraft::Window::INVENTORY_HOTBAR_START + 7);
        break;
      case Botcraft::ToolType::Shears:
        // put shears at slot 42
        taskSrc.push_back(tools[0].id);
        taskDst.push_back(Botcraft::Window::INVENTORY_HOTBAR_START + 6);
        break;
      case Botcraft::ToolType::Shovel:
        // put shovel at slot 41
        taskSrc.push_back(tools[0].id);
        taskDst.push_back(Botcraft::Window::INVENTORY_HOTBAR_START + 5);
        break;
      default:
        std::cout << GetTime() << "Unexpected Botcraft::ToolType..."
                  << std::endl;
        break;
    }
  }

  // Move tool to desire slot.
  for (int i = 0; i < taskSrc.size(); i++) {
    std::cout << taskSrc[i] << ", " << taskDst[i] << std::endl;
    if (taskSrc[i] == taskDst[i]) continue;
    Botcraft::Status s = SwapItemsInContainer(
        c, Botcraft::Window::PLAYER_INVENTORY_INDEX, taskSrc[i], taskDst[i]);
    if (s == Botcraft::Status::Failure) {
      std::cout << "Swap (" << taskSrc[i] << ", " << taskDst[i] << ") fail..."
                << std::endl;
    }

    // If src and dst overlaps, swap will give wrong result.
    // If no adjustment, item in slot A will moved to slot C instead of slot B.
    // (A, B), (B, C) -> (A, B), (A, C)
    for (int j = i + 1; j < taskSrc.size(); j++) {
      if (taskSrc[j] == taskDst[i]) taskSrc[j] = taskSrc[i];
    }
  }

  return Botcraft::Status::Success;
}

// Dump everything to recycle chest.
// Player is src, recycle chest is dst.
Botcraft::Status DumpItems(Botcraft::BehaviourClient &c) {
  std::cout << GetTime() << "Trying to dump items to recycle chest..."
            << std::endl;
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::InventoryManager> inventory_manager =
      c.GetInventoryManager();
  std::vector<Botcraft::Position> chests = artist.conf.chests["recycle"];

  for (auto chest : chests) {
    if (FindPathAndMove(c, chest, 2, 2, 0, 6, 2, 2, 2, 2, 0, 3, 2, 2) ==
        Botcraft::Status::Failure)
      continue;
    std::cout << GetTime() << "dumping items..." << std::endl;
    if (OpenContainer(c, chest) == Botcraft::Status::Failure) {
      std::cout << GetTime() << "Open Chest " << chest << " Error" << std::endl;
      continue;
    }

    std::queue<int16_t> slotSrc, slotDst;
    int16_t containerId, firstPlayerIndex;

    // Find possible swaps
    containerId = inventory_manager->GetFirstOpenedWindowId();
    if (containerId == -1) continue;

    std::shared_ptr<Botcraft::Window> container =
        inventory_manager->GetWindow(containerId);
    firstPlayerIndex = container->GetFirstPlayerInventorySlot();

    const std::map<int16_t, ProtocolCraft::Slot> &slots = container->GetSlots();

    for (auto it = slots.begin(); it != slots.end(); ++it) {
      if (it->first >= 0 && it->first < firstPlayerIndex &&
          it->second.IsEmptySlot()) {
        slotDst.push(it->first);
      } else if (it->first >= firstPlayerIndex &&
                 it->first <= firstPlayerIndex + 31 &&
                 !it->second.IsEmptySlot()) {
        // we put player's tool from firstPlayerIndex+32 to firstPlayerIndex+35
        slotSrc.push(it->first);
      }
    }

    while (!slotSrc.empty() && !slotDst.empty()) {
      std::cout << GetTime() << "Swap " << slotSrc.front() << " and "
                << slotDst.front() << std::endl;
      if (SwapItemsInContainer(c, containerId, slotSrc.front(),
                               slotDst.front()) == Botcraft::Status::Failure) {
        CloseContainer(c, containerId);
        std::cout << GetTime() << "Error when trying to dump items to chest..."
                  << std::endl;
      }
      slotSrc.pop();
      slotDst.pop();
    }

    // Close the chest
    CloseContainer(c, containerId);
    if (slotSrc.empty()) break;
  }
  std::cout << GetTime() << "Finish dumping items..." << std::endl;

  Botcraft::Status s = checkInventoryAllClear(c);
  if (s == Botcraft::Status::Failure) {
    std::cout << GetTime() << "Early stop due to recycle chest full..."
              << std::endl;
    MessageOutput("Early stop due to recycle chest full...", &artist);
    ListPlayerInventory(&artist);
    return Botcraft::Status::Failure;
  }

  return Botcraft::Status::Success;
}

Botcraft::Status TaskPrioritize(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::string algo = artist.conf.algo.method;
  artist.board.Set(KeyTaskQueued, true);

  if (algo == AlgoSliceDFS) {
    SliceDFS(c);
  } else {
    std::cout << GetTime() << "Get unrecognized prioritize method: " << algo
              << std::endl;
    return Botcraft::Status::Failure;
  }

  return Botcraft::Status::Success;
}

Botcraft::Status CollectAllMaterial(Botcraft::BehaviourClient &c) {
  std::cout << GetTime() << "Trying to collect material..." << std::endl;
  Artist &artist = static_cast<Artist &>(c);
  std::map<std::string, int, MaterialCompare> itemCounter =
      artist.board.Get<std::map<std::string, int, MaterialCompare>>(
          KeyItemCounter);

  for (auto item : itemCounter) {
    Botcraft::Status s = CollectSingleMaterial(c, item.first, item.second);
    if (s == Botcraft::Status::Failure) {
      std::cout << "Early stop due to collect material fail..." << std::endl;
      MessageOutput("Early stop due to collect material fail...", &artist);
      ListPlayerInventory(&artist);
      return Botcraft::Status::Failure;
    }
  }
  return Botcraft::Status::Success;
}

Botcraft::Status CollectSingleMaterial(Botcraft::BehaviourClient &c,
                                       std::string itemName, int needed) {
  std::cout << GetTime() << "Collecting " << itemName << " for " << needed
            << std::endl;

  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::InventoryManager> inventory_manager =
      c.GetInventoryManager();
  std::vector<Botcraft::Position> allChests = artist.conf.chests[itemName];

  bool get_all_material = false;
  int remain_empty_slot = -1;
  for (auto chest : allChests) {
    std::cout << GetTime() << "========== CHEST ==========" << std::endl;
    SortInventory(c);
    Botcraft::Status moveResult =
        FindPathAndMove(c, chest, 2, 2, 2, 4, 2, 2, 0, 0, -1, 0, 0, 0);
    if (moveResult == Botcraft::Status::Failure) {
      std::cout << GetTime() << "Go to chest fail..." << std::endl;
      continue;
    }
    if (OpenContainer(c, chest) == Botcraft::Status::Failure) {
      std::cout << GetTime() << "Interact with chest fail..." << std::endl;
      continue;
    }

    int _need = needed;
    std::queue<int16_t> canTake, canPut;
    const int16_t containerId = inventory_manager->GetFirstOpenedWindowId();
    std::shared_ptr<Botcraft::Window> container =
        inventory_manager->GetWindow(containerId);

    {
      std::vector<std::pair<int16_t, ProtocolCraft::Slot>> _canPut, _canTake;
      const int16_t playerInvStart = container->GetFirstPlayerInventorySlot();

      for (auto slot : container->GetSlots()) {
        if (slot.first < playerInvStart && !slot.second.IsEmptySlot()) {
          _canTake.push_back(slot);
        } else if (slot.first >= playerInvStart && !slot.second.IsEmptySlot()) {
          std::string _name = Botcraft::AssetsManager::getInstance()
                                  .GetItem(slot.second.GetItemID())
                                  ->GetName();
          if (_name == itemName && slot.second.GetItemCount() < 64)
            _canPut.push_back(slot);
          if (_name == itemName && slot.second.GetItemCount() == 64)
            _need -= 64;
        } else if (slot.first >= playerInvStart && slot.second.IsEmptySlot()) {
          _canPut.push_back(slot);
        }
      }

      sort(_canPut.begin(), _canPut.end(),
           [](const std::pair<const int16_t, ProtocolCraft::Slot> &a,
              const std::pair<const int16_t, ProtocolCraft::Slot> &b) {
             return a.second.GetItemCount() > b.second.GetItemCount();
           });
      for (auto slot : _canPut) {
        canPut.push(slot.first);
      }

      sort(_canTake.begin(), _canTake.end(),
           [](const std::pair<const int16_t, ProtocolCraft::Slot> &a,
              const std::pair<const int16_t, ProtocolCraft::Slot> &b) {
             return a.second.GetItemCount() > b.second.GetItemCount();
           });
      for (auto slot : _canTake) {
        canTake.push(slot.first);
      }
    }

    while (!canTake.empty() && !canPut.empty() && !get_all_material) {
      std::cout << GetTime() << "Swap from ID " << canTake.front() << " to ID "
                << canPut.front() << std::endl;
      Botcraft::Status swapResult =
          SwapItemsInContainer(c, containerId, canTake.front(), canPut.front());
      if (swapResult ==
          Botcraft::Status::Failure) {  // if failed, wait for a while and retry
        Botcraft::Utilities::SleepFor(std::chrono::milliseconds(500));
        std::cout << GetTime() << "Take " << itemName << " Failed" << std::endl;
        continue;
      }
      int took_amount = container->GetSlot(canPut.front()).GetItemCount();
      std::cout << GetTime() << "Take " << itemName << " for " << took_amount
                << std::endl;
      _need -= took_amount;
      canPut.pop();
      canTake.pop();

      if (_need < 1) get_all_material = true;
    }

    std::cout << GetTime() << "========== LIST ==========" << std::endl;
    {
      const int16_t playerInvStart = container->GetFirstPlayerInventorySlot();
      remain_empty_slot = 0;
      for (auto p : container->GetSlots()) {
        if (p.first >= playerInvStart) {
          if (p.second.IsEmptySlot()) {
            remain_empty_slot++;
            std::cout << GetTime() << "Slot" << p.first << ": " << std::endl;
          } else {
            std::cout << GetTime() << "Slot " << p.first << ": "
                      << Botcraft::AssetsManager::getInstance()
                             .Items()
                             .at(p.second.GetItemID())
                             ->GetName()
                      << " x " << static_cast<int>(p.second.GetItemCount())
                      << std::endl;
          }
        }
      }
    }
    std::cout << GetTime() << "======= LIST CLOSE =======" << std::endl;

    CloseContainer(c, containerId);
    std::cout << GetTime() << "======= CHEST CLOSE =======" << std::endl;
    if (get_all_material) break;
  }

  if (!get_all_material) {
    if (remain_empty_slot == 0) {
      std::cout << GetTime() << "Inventory might be full..." << std::endl;
      MessageOutput("Inventory might be full...", &artist);
    } else {
      std::cout << GetTime() << itemName << " might not enough..." << std::endl;
      MessageOutput(itemName + " might not enough...", &artist);
    }
    return Botcraft::Status::Failure;
  }

  return Botcraft::Status::Success;
}

Botcraft::Status TaskExecutor(Botcraft::BehaviourClient &c) {
  std::cout << GetTime() << "Execute task in queue..." << std::endl;
  Artist &artist = static_cast<Artist &>(c);
  std::queue<Botcraft::Position> qTaskPosition =
      artist.board.Get<std::queue<Botcraft::Position>>(KeyTaskPosQ);
  std::queue<std::string> qTaskType =
      artist.board.Get<std::queue<std::string>>(KeyTaskTypeQ);
  std::queue<std::string> qTaskName =
      artist.board.Get<std::queue<std::string>>(KeyTaskNameQ);
  int retry_times = artist.conf.algo.retry;
  std::vector<Botcraft::Position> offsets{
      Botcraft::Position(1, 0, 0), Botcraft::Position(-1, 0, 0),
      Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1),
      Botcraft::Position(2, 0, 0), Botcraft::Position(-2, 0, 0),
      Botcraft::Position(0, 0, 2), Botcraft::Position(0, 0, -2)};

  if (!qTaskPosition.empty() && !qTaskType.empty() && !qTaskName.empty()) {
    std::cout << GetTime() << "Remain " << qTaskPosition.size() << " tasks..."
              << std::endl;
    Botcraft::Position taskPos = qTaskPosition.front();
    qTaskPosition.pop();
    std::string taskType = qTaskType.front();
    qTaskType.pop();
    std::string blockName = qTaskName.front();
    qTaskName.pop();
    for (int i = 0; i < retry_times; i++) {
      if (GetItemAmount(c, blockName) == 0 && taskType == "Place") {
        // Item not enough
        // Directly clear all tasks
        std::cout << GetTime() << "Item " << blockName << " not enough, return."
                  << std::endl;
        qTaskPosition = std::queue<Botcraft::Position>();
        qTaskType = std::queue<std::string>();
        qTaskName = std::queue<std::string>();
        break;
      }
      Botcraft::Status exec_result =
          ExecuteTask(c, taskType, taskPos, blockName);
      if (exec_result == Botcraft::Status::Success) {
        break;
      } else {
        auto nextPos = taskPos + offsets[i % offsets.size()];
        const Botcraft::Blockstate *block = c.GetWorld()->GetBlock(nextPos);
        if (block != nullptr && !block->IsAir()) {  // simple detect
          std::cout << GetTime()
                    << "Task fail, move to another position and try again ("
                    << i << ")..." << std::endl;
          FindPathAndMove(c, nextPos, 0, 0, 3, 3, 0, 0, -1, -1, -1, -1, -1, -1);
        }
      }
    }

    artist.board.Set(KeyTaskPosQ, qTaskPosition);
    artist.board.Set(KeyTaskTypeQ, qTaskType);
    artist.board.Set(KeyTaskNameQ, qTaskName);

    // Still has task in queue, return fail.
    return Botcraft::Status::Failure;
  } else {
    artist.board.Set(KeyTaskQueued, false);

    // All tasks resolved, return success.
    return Botcraft::Status::Success;
  }
}

Botcraft::Status ExecuteTask(Botcraft::BehaviourClient &c, std::string action,
                             Botcraft::Position blockPos,
                             std::string blockName) {
  std::cout << GetTime() << "Task:" << std::setw(5) << action
            << ", Block Name:" << std::setw(32) << blockName
            << ", Botcraft::Position:" << blockPos << std::endl;

  if (FindPathAndMove(c, blockPos, 3, 3, 3, 3, 3, 3, 0, 0, 0, 2, 0, 0) ==
      Botcraft::Status::Failure) {
    std::cout << GetTime() << "Move Error" << std::endl;
    return Botcraft::Status::Failure;
  }
  std::string bn = GetWorldBlock(c, blockPos);
  if (action == "Dig") {
    if (bn == "minecraft:air")
      return Botcraft::Status::Success;
    else
      return Dig(c, blockPos, true);
  } else if (action == "Place") {
    if (bn == "minecraft:air") {
      PlaceBlock(c, blockName, blockPos, std::nullopt, true, true);
      RemoveNeighborExtraBlock(c, blockPos);
      return Botcraft::Status::Success;
    } else if (bn != blockName) {
      Dig(c, blockPos, true);
      return Botcraft::Status::Failure;
    } else {
      return Botcraft::Status::Success;
    }
  }

  std::cout << GetTime() << "Unknown task in ExecuteNextTask..." << std::endl;
  return Botcraft::Status::Failure;
}

Botcraft::Status RemoveNeighborExtraBlock(Botcraft::BehaviourClient &c,
                                          Botcraft::Position blockPos) {
  Artist &artist = static_cast<Artist &>(c);
  const Botcraft::Position &anchor = artist.conf.nbt.anchor;
  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);
  std::vector<Botcraft::Position> offsets{
      Botcraft::Position(1, 0, 0), Botcraft::Position(-1, 0, 0),
      Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0),
      Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1)};

  for (int i = 0; i < offsets.size(); i++) {
    Botcraft::Position newPos = blockPos + offsets[i];
    Botcraft::Position relativePos = newPos - anchor;
    bool xCheck = newPos.x >= start.x && newPos.x <= end.x;
    bool yCheck = newPos.y >= start.y && newPos.y <= end.y;
    bool zCheck = newPos.z >= start.z && newPos.z <= end.z;
    if (xCheck && yCheck && zCheck) {
      int16_t blockID = target[relativePos.x][relativePos.y][relativePos.z];
      std::string idealBlock = palette.at(blockID);
      std::string worldBlock = GetWorldBlock(c, newPos);
      if (idealBlock != worldBlock) Dig(c, newPos);
    }
  }

  return Botcraft::Status::Success;
}

Botcraft::Status FindPathAndMove(Botcraft::BehaviourClient &c,
                                 Botcraft::Position pos, int x_tol_pos,
                                 int x_tol_neg, int y_tol_pos, int y_tol_neg,
                                 int z_tol_pos, int z_tol_neg, int excl_x_pos,
                                 int excl_x_neg, int excl_y_pos, int excl_y_neg,
                                 int excl_z_pos, int excl_z_neg) {
  try {
    pf::Position to{pos.x, pos.y, pos.z};
    if (excl_x_pos >= 0 || excl_x_neg >= 0 || excl_y_pos >= 0 ||
        excl_y_neg >= 0 || excl_z_pos >= 0 || excl_z_neg >= 0) {
      using RGoal = pf::goal::RangeGoal<pf::Position>;
      using EGoal = pf::goal::ExclusiveGoal<RGoal>;
      using CGoal = pf::goal::CombineGoal<RGoal, EGoal>;
      CGoal goal(RGoal(to, x_tol_pos, x_tol_neg, y_tol_pos, y_tol_neg,
                       z_tol_pos, z_tol_neg),
                 EGoal(RGoal(to, excl_x_pos, excl_x_neg, excl_y_pos, excl_y_neg,
                             excl_z_pos, excl_z_neg)));
      return FindPathAndMoveImpl(c, pos, goal);
    } else {
      pf::goal::RangeGoal<pf::Position> goal(
          to, x_tol_pos, x_tol_neg, y_tol_pos, y_tol_neg, z_tol_pos, z_tol_neg);
      return FindPathAndMoveImpl(c, pos, goal);
    }
  } catch (const std::exception &e) {
    std::cerr << "Move Fatal Error" << std::endl;
    std::cerr << e.what() << std::endl;
    return Botcraft::Status::Failure;
  }
}

Botcraft::Status FindPathAndMoveImpl(Botcraft::BehaviourClient &c,
                                     Botcraft::Position pos,
                                     pf::goal::GoalBase<pf::Position> &goal) {
  Artist &artist = static_cast<Artist &>(c);
  auto finder = artist.finder;

  auto getFromPosition = [&]() -> pf::Position {
    pf::Position from;
    // get player location
    std::shared_ptr<Botcraft::LocalPlayer> local_player =
        c.GetEntityManager()->GetLocalPlayer();
    auto player_pos = local_player->GetPosition();
    from.x = static_cast<int>(floor(player_pos.x));
    from.y = static_cast<int>(floor(player_pos.y + 0.25)) - 1;
    from.z = static_cast<int>(floor(player_pos.z));
    return from;
  };

  pf::Position to{pos.x, pos.y, pos.z};
  bool r = false;
  for (int i = 0; i < 2; ++i) {
    pf::Position from = getFromPosition();
    std::cout << GetTime() << "Find a path from " << from << " to " << to
              << "\n";
    // find path and go
    r = finder.findPathAndGo(from, goal, 15000);
    if (r) break;
    from = getFromPosition();  // get the latest position
    std::cout << GetTime() << "Failed, retry after 5 seconds..." << std::endl;
    Botcraft::Utilities::SleepFor(std::chrono::seconds(3));  // delay 3 seconds
  }

  if (!r) {
    std::cout << GetTime() << "Bot get stuck, try to teleport..." << std::endl;
    Botcraft::Utilities::SleepFor(std::chrono::seconds(5));  // delay 5 seconds
    std::string homeCommand = artist.conf.other.home;
    std::cout << GetTime() << "Send TP command..." << std::endl;
    c.SendChatCommand(homeCommand);
    std::cout << GetTime() << "Wait for TP success..." << std::endl;

    // wait for 10 seconds
    if (static_cast<Artist &>(c).waitTP(std::chrono::seconds(10)) ==
        false) {  // false
      std::cout << GetTime() << "TP Failed..." << std::endl;
      return Botcraft::Status::Failure;
    }
    std::cout << GetTime() << "TP Success!!!" << std::endl;

    std::cout << GetTime() << "World loading..." << std::endl;
    WaitServerLoad(c);  // always return true
    std::cout << GetTime() << "Finish world loading" << std::endl;

    // update player's new position
    pf::Position from = getFromPosition();
    std::cout << GetTime() << "Find a path from " << from << " to " << to
              << "\n";
    r = finder.findPathAndGo(from, goal, 15000);
  }

  return (r ? Botcraft::Status::Success : Botcraft::Status::Failure);
}

/*
If everything is correct, return Success, otherwise return Failure.
*/
Botcraft::Status checkCompletion(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::World> world = c.GetWorld();
  Botcraft::Position anchor = artist.conf.nbt.anchor;

  Botcraft::Position target_pos, world_pos;

  std::vector<std::vector<std::vector<BlockMemo>>> mapMemory =
      artist.board.Get<std::vector<std::vector<std::vector<BlockMemo>>>>(
          KeyMapMemo);

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);

  Botcraft::Status isComplete = Botcraft::Status::Success;
  int workers = artist.board.Get<int>(KeyWorkerCount, 1);
  int col = artist.board.Get<int>(KeyWorkerCol, 0);

  for (int x = start.x; x <= end.x; x++) {
    if ((x - start.x) % workers != col) continue;

    world_pos.x = x;
    target_pos.x = x - start.x;
    for (int y = start.y; y <= end.y; y++) {
      world_pos.y = y;
      target_pos.y = y - start.y;
      for (int z = start.z; z <= end.z; z++) {
        world_pos.z = z;
        target_pos.z = z - start.z;

        const int16_t target_id =
            target[target_pos.x][target_pos.y][target_pos.z];
        std::string target_name = palette.at(target_id);

        std::string block_name = "minecraft:air";
        const Botcraft::Blockstate *block = world->GetBlock(world_pos);
        if (!world->IsLoaded(world_pos)) {
          continue;
        } else if (block) {
          block_name = block->GetName();
        }

        mapMemory[target_pos.x][target_pos.y][target_pos.z].name = block_name;
        if (block_name == "minecraft:air" && target_name == "minecraft:air") {
          // continue if it is a air block
          continue;
        } else if (block_name == "minecraft:air" &&
                   target_name != "minecraft:air") {
          // Found air in real world, but it should be something else
          isComplete = Botcraft::Status::Failure;
          mapMemory[target_pos.x][target_pos.y][target_pos.z].match = false;
        } else if (block_name != "minecraft:air" &&
                   target_name == "minecraft:air") {
          // Found something else, but it should be air.
          isComplete = Botcraft::Status::Failure;
          mapMemory[target_pos.x][target_pos.y][target_pos.z].match = false;
        } else if (block_name != target_name) {
          // The name of block not match.
          isComplete = Botcraft::Status::Failure;
          mapMemory[target_pos.x][target_pos.y][target_pos.z].match = false;
        }
      }
    }
  }

  artist.board.Set(KeyMapMemo, mapMemory);
  return isComplete;
}

void updateDiscordStatus(Artist *artist) {
  if (!artist->conf.priv.discordEnable) return;

  DiscordBot &b = DiscordBot::getDiscordBot();
  int ratio;
  double percent;
  std::tie(ratio, percent) = CalRatioAndPercent(artist);
  b.setDCStatus(std::to_string(percent) + " %");
  return;
}

Botcraft::Status CheckCompletion(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::World> world = c.GetWorld();
  Botcraft::Position anchor = artist.conf.nbt.anchor;

  Botcraft::Position target_pos, world_pos;

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const Botcraft::Position size = end - start + Botcraft::Position(1, 1, 1);
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);

  std::vector<Botcraft::Position> checkpoints{
      Botcraft::Position(static_cast<int>(size.x * 0.3), 0,
                         static_cast<int>(size.z * 0.3)),
      Botcraft::Position(static_cast<int>(size.x * 0.6), 0,
                         static_cast<int>(size.z * 0.3)),
      Botcraft::Position(static_cast<int>(size.x * 0.3), 0,
                         static_cast<int>(size.z * 0.6)),
      Botcraft::Position(static_cast<int>(size.x * 0.6), 0,
                         static_cast<int>(size.z * 0.6))};

  // initialize map recorder
  // default value will set to true, if the block is incorrect will set to false
  std::vector<std::vector<std::vector<BlockMemo>>> mapMemory(
      size.x, std::vector(size.y, std::vector(size.z, BlockMemo{})));
  artist.board.Set(KeyMapMemo, mapMemory);

  const bool log_details = false;
  const bool log_errors = true;
  const bool full_check = true;

  Botcraft::Status isComplete = Botcraft::Status::Success;
  for (auto cp : checkpoints) {
    std::cout << GetTime() << "Check checkpoint..." << std::endl;
    Botcraft::Status moveResult = FindPathAndMove(c, anchor + cp, 0, 0, 5, 5, 0,
                                                  0, -1, -1, -1, -1, -1, -1);
    if (moveResult == Botcraft::Status::Failure) {
      std::cout << GetTime() << "Move to checkpoint fail..." << std::endl;
    }
    if (checkCompletion(c) == Botcraft::Status::Failure)
      isComplete = Botcraft::Status::Failure;
  }

  // update xCheck
  mapMemory =
      artist.board.Get<std::vector<std::vector<std::vector<BlockMemo>>>>(
          KeyMapMemo);
  std::vector<bool> xCheck = std::vector(size.x, false);

  for (int x = 0; x < size.x; x++) {
    bool isAllDone = true;
    for (int y = 0; y < size.y; y++) {
      for (int z = 0; z < size.z; z++) {
        if (!mapMemory[x][y][z].match) isAllDone = false;
      }
    }

    xCheck[x] = isAllDone;
  }

  artist.board.Set(KeyXCheck, xCheck);
  updateDiscordStatus(&artist);

  return isComplete;
}

Botcraft::Status WarnConsole(Botcraft::BehaviourClient &c,
                             const std::string &msg) {
  std::cout << GetTime() << "[" << c.GetNetworkManager()->GetMyName()
            << "]: " << msg << std::endl;
  return Botcraft::Status::Success;
}

Botcraft::Status LoadNBT(Botcraft::BehaviourClient &c) {
  std::cout << GetTime() << "Loading NBT file..." << std::endl;
  ProtocolCraft::NBT::Value loaded_file;
  Artist &artist = static_cast<Artist &>(c);
  Botcraft::Position offset = artist.conf.nbt.anchor;
  std::string temp_block = artist.conf.nbt.tmpBlock;
  std::string nbt_path = artist.conf.nbt.name;

  try {
    std::ifstream infile(nbt_path, std::ios::binary);
    infile.unsetf(std::ios::skipws);
    infile >> loaded_file;
    infile.close();
  } catch (const std::exception &e) {
    std::cout << GetTime() << "Early stop due to loading NBT file fail, "
              << e.what() << std::endl;
    return Botcraft::Status::Failure;
  }

  std::map<int16_t, std::string> palette;
  palette[-1] = "minecraft:air";
  int16_t id_temp_block = -1;
  std::map<int16_t, int> num_blocks_used;

  if (!loaded_file.contains("palette") ||
      !loaded_file["palette"].is_list_of<ProtocolCraft::NBT::TagCompound>()) {
    std::cout << GetTime()
              << "Early stop due to loading NBT file fail, no palette "
                 "TagCompound found"
              << std::endl;
    return Botcraft::Status::Failure;
  }

  const std::vector<ProtocolCraft::NBT::TagCompound> &palette_list =
      loaded_file["palette"].as_list_of<ProtocolCraft::NBT::TagCompound>();
  for (int i = 0; i < palette_list.size(); ++i) {
    const std::string &block_name = palette_list[i]["Name"].get<std::string>();
    palette[i] = block_name;
    num_blocks_used[i] = 0;
    if (block_name == temp_block) id_temp_block = i;
  }

  Botcraft::Position min(std::numeric_limits<int>().max(),
                         std::numeric_limits<int>().max(),
                         std::numeric_limits<int>().max());
  Botcraft::Position max(std::numeric_limits<int>().min(),
                         std::numeric_limits<int>().min(),
                         std::numeric_limits<int>().min());

  if (!loaded_file.contains("blocks") ||
      !loaded_file["blocks"].is_list_of<ProtocolCraft::NBT::TagCompound>()) {
    std::cout << GetTime()
              << "Early stop due to loading NBT file fail, no blocks "
                 "TagCompound found"
              << std::endl;
    return Botcraft::Status::Failure;
  }

  const std::vector<ProtocolCraft::NBT::TagCompound> &block_tag =
      loaded_file["blocks"].as_list_of<ProtocolCraft::NBT::TagCompound>();
  for (const auto &c : block_tag) {
    const std::vector<int> &pos_list = c["pos"].as_list_of<int>();
    const int x = pos_list[0];
    const int y = pos_list[1];
    const int z = pos_list[2];

    if (x < min.x) min.x = x;
    if (y < min.y) min.y = y;
    if (z < min.z) min.z = z;
    if (x > max.x) max.x = x;
    if (y > max.y) max.y = y;
    if (z > max.z) max.z = z;
  }

  Botcraft::Position size = max - min + Botcraft::Position(1, 1, 1);
  Botcraft::Position start = offset;
  Botcraft::Position end = offset + size - Botcraft::Position(1, 1, 1);

  std::cout << GetTime() << "Start: " << start << " | "
            << "End: " << end << std::endl;

  // Fill the target area with air (-1)
  std::vector<std::vector<std::vector<int16_t>>> target(
      size.x, std::vector<std::vector<int16_t>>(
                  size.y, std::vector<int16_t>(size.z, -1)));

  // Read all block to place
  for (const auto &c : block_tag) {
    const int state = c["state"].get<int>();
    const std::vector<int> &pos_list = c["pos"].as_list_of<int>();
    const int x = pos_list[0];
    const int y = pos_list[1];
    const int z = pos_list[2];

    target[x - min.x][y - min.y][z - min.z] = state;
    num_blocks_used[state] += 1;
  }

  if (id_temp_block == -1) {
    std::cout << GetTime() << "Can't find the given temp block " << temp_block
              << " in the palette" << std::endl;
  } else {
    int removed_layers = 0;
    // Check the bottom Y layers, if only
    // air or temp block, the layer can be removed
    while (true) {
      bool is_removable = true;
      int num_temp_block = 0;
      for (int x = 0; x < size.x; ++x) {
        for (int z = 0; z < size.z; z++) {
          if (target[x][0][z] == id_temp_block) num_temp_block += 1;

          if (target[x][0][z] != -1 && target[x][0][z] != id_temp_block) {
            is_removable = false;
            break;
          }
          if (!is_removable) break;
        }
      }

      if (!is_removable) break;

      for (int x = 0; x < size.x; ++x) {
        target[x].erase(target[x].begin());
      }
      num_blocks_used[id_temp_block] -= num_temp_block;
      removed_layers++;
      size.y -= 1;
      end.y -= 1;
    }

    std::cout << GetTime() << "Removed the bottom " << removed_layers
              << " layer" << (removed_layers > 1 ? "s" : "") << std::endl;
  }

  std::cout << GetTime() << "Total size: " << size << std::endl;

  std::stringstream needed;
  needed << "Block needed:\n";
  for (auto it = num_blocks_used.begin(); it != num_blocks_used.end(); ++it) {
    needed << std::setw(35) << palette[it->first] << "----" << it->second
           << "\n";
  }
  std::cout << GetTime() << needed.rdbuf() << std::endl;

  // Check if some block can't be placed (flying blocks)
  std::stringstream flyings;
  flyings << "Flying blocks, you might have to place them yourself:\n";
  Botcraft::Position target_pos;

  const std::vector<Botcraft::Position> neighbour_offsets(
      {Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0),
       Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1),
       Botcraft::Position(1, 0, 0), Botcraft::Position(-1, 0, 0)});

  for (int x = 0; x < size.x; x++) {
    target_pos.x = x;
    // If this block is on the floor, it's ok
    for (int y = 1; y < size.y; y++) {
      target_pos.y = y;
      for (int z = 0; z < size.z; z++) {
        target_pos.z = z;

        const int16_t target_id =
            target[target_pos.x][target_pos.y][target_pos.z];

        if (target_id != -1) {
          // Check all target neighbours
          bool has_neighbour = false;
          for (int i = 0; i < neighbour_offsets.size(); i++) {
            const Botcraft::Position neighbour_pos =
                target_pos + neighbour_offsets[i];

            bool x_constrain = neighbour_pos.x >= 0 && neighbour_pos.x < size.x;
            bool y_constrain = neighbour_pos.y >= 0 && neighbour_pos.y < size.y;
            bool z_constrain = neighbour_pos.z >= 0 && neighbour_pos.z < size.z;
            if (x_constrain && y_constrain && z_constrain &&
                target[neighbour_pos.x][neighbour_pos.y][neighbour_pos.z] !=
                    -1) {
              has_neighbour = true;
              break;
            }
          }

          if (!has_neighbour) {
            flyings << start + target_pos << "\t" << palette[target_id] << "\n";
          }
        }
      }
    }
  }
  std::cout << GetTime() << flyings.rdbuf() << std::endl;

  artist.board.Set(KeyNbtStart, start);
  artist.board.Set(KeyNbtEnd, end);
  artist.board.Set(KeyNbtTarget, target);
  artist.board.Set(KeyNbtPalette, palette);
  artist.board.Set(KeyNBTLoaded, true);

  return Botcraft::Status::Success;
}

Botcraft::Status EatUntilFull(Botcraft::BehaviourClient &c, std::string food) {
  Artist &artist = static_cast<Artist &>(c);
  while (IsHungry(c, 20) == Botcraft::Status::Success) {
    std::shared_ptr<Botcraft::InventoryManager> inventory_manager =
        c.GetInventoryManager();
    const ProtocolCraft::Slot &slot =
        inventory_manager->GetPlayerInventory()->GetSlot(
            Botcraft::Window::INVENTORY_OFFHAND_INDEX);
    std::string name = Botcraft::AssetsManager::getInstance()
                           .Items()
                           .at(slot.GetItemID())
                           ->GetName();
    std::cout << GetTime() << "Left hand: " << name << std::endl;
    Botcraft::Status r = Eat(c, food, true);
    ListPlayerInventory(&artist);
    if (r == Botcraft::Status::Failure) return Botcraft::Status::Failure;
  }

  return Botcraft::Status::Success;
}
