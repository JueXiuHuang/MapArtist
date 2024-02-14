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
#include <botcraft/Utilities/SleepUtilities.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include "CustomTask.hpp"
#include "Algorithm.hpp"
#include "PathFinding.hpp"
#include "Utils.hpp"
#include "Artist.hpp"

using namespace Botcraft;
using namespace ProtocolCraft;

Status WaitServerLoad(BehaviourClient& c) {
  std::shared_ptr<LocalPlayer> local_player = c.GetEntityManager()->GetLocalPlayer();
  Utilities::WaitForCondition([&]() {
    return local_player->GetPosition().y < 1000;
  }, 10000);

  return Status::Success;
}

Status checkInventoryAllClear(BehaviourClient& c) {
  std::shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  std::shared_ptr<Window> playerInv = inventory_manager->GetPlayerInventory();
  for (short i = Window::INVENTORY_STORAGE_START; i < Window::INVENTORY_HOTBAR_START+5; i++) {
    const Slot& slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) continue;
    std::string itemName = AssetsManager::getInstance().Items().at(slot.GetItemID())->GetName();
    if (itemName.find("_pickaxe") != std::string::npos) {
      continue;
    }
    if (itemName.find("_axe") != std::string::npos) {
      continue;
    }
    if (itemName.find("_shovel") != std::string::npos) {
      continue;
    }
    if (itemName.find("shears") != std::string::npos) {
      continue;
    }
    
    // Something else in player's inventory
    return Status::Failure;
  }

  // Player's inventory all clear
  return Status::Success;
}

Status GetFood(BehaviourClient& c, const std::string& food_name) {
  std::shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();

  // Sort the chest and make sure the first slot in hotbar is empty
  // Food will place in this slot
  SwapItemsInContainer(c, Window::PLAYER_INVENTORY_INDEX,
                          Window::INVENTORY_HOTBAR_START, 
                          Window::INVENTORY_OFFHAND_INDEX);

  const std::vector<Position>& chests = c.GetBlackboard().Get<std::vector<Position>>("chest:" + food_name);

  std::vector<std::size_t> chests_indices(chests.size());
  for (std::size_t i = 0; i < chests.size(); ++i) {
    chests_indices[i] = i;
  }

  short container_id;
  bool item_taken = false;

  for (std::size_t index = 0; index < chests.size(); ++index) {
    const std::size_t i = chests_indices[index];
    // If we can't open this chest for a reason
    FindPathAndMove(c, chests[i],  1, 1, 1, 1, 1, 1,  0, 0, 0, 0, 0, 0);
    if (OpenContainer(c, chests[i]) == Status::Failure) continue;

    short player_dst = -1;
    while (true) {
      std::vector<short> slots_src;
      container_id = inventory_manager->GetFirstOpenedWindowId();
      if (container_id == -1) continue;

      const std::shared_ptr<Window> container = inventory_manager->GetWindow(container_id);

      const short playerFirstSlot = container->GetFirstPlayerInventorySlot();
      player_dst = playerFirstSlot + 9 * 3;

      const std::map<short, Slot>& slots = container->GetSlots();

      slots_src.reserve(slots.size());

      for (auto it = slots.begin(); it != slots.end(); ++it) {
        // Chest is src
        if (it->first >= 0 && it->first < playerFirstSlot && !it->second.IsEmptySlot() &&
            AssetsManager::getInstance().Items().at(it->second.GetItemID())->GetName() == food_name)
          slots_src.push_back(it->first);
      }

      if (slots_src.size() > 0) {
        std::cout << GetTime() << "Found food in chest." << std::endl;
        const int src_index = 0;
        // Try to swap the items
        if (SwapItemsInContainer(c, container_id, slots_src[src_index], player_dst) == Status::Success) {
          std::cout << GetTime() << "Get food success." << std::endl;
          item_taken = true;
          break;
        }
      }
      // This means the chest doesn't have any food
      else break;
    }

    CloseContainer(c, container_id);

    if (!item_taken) continue;

    // No need to continue loooking in the other chests
    break;
  }

  return item_taken ? Status::Success : Status::Failure;
}

Status SortChestWithDesirePlace(BehaviourClient& c) {
  std::shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  std::shared_ptr<Window> playerInv = inventory_manager->GetPlayerInventory();
  Status state = SortInventory(c);
  std::queue<short> taskSrc, taskDst;
  
  for (short i = Window::INVENTORY_STORAGE_START; i < Window::INVENTORY_HOTBAR_START+5; i++) {
    const Slot& slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) continue;
    std::string itemName = AssetsManager::getInstance().Items().at(slot.GetItemID())->GetName();
    if (itemName.find("_pickaxe") != std::string::npos) {
      // put pickaxe at slot 44
      taskSrc.push(i);
      taskDst.push(44);
      continue;
    }
    if (itemName.find("_axe") != std::string::npos) {
      // put axe at slot 43
      taskSrc.push(i);
      taskDst.push(43);
      continue;
    }
    if (itemName.find("_shovel") != std::string::npos) {
      // put shovel at slot 42
      taskSrc.push(i);
      taskDst.push(42);
      continue;
    }
    if (itemName.find("shears") != std::string::npos) {
      // put shears at slot 41
      taskSrc.push(i);
      taskDst.push(41);
      continue;
    }
  }

  while (!taskSrc.empty() && !taskDst.empty()) {
    SwapItemsInContainer(c, Window::PLAYER_INVENTORY_INDEX, taskSrc.front(), taskDst.front());
    taskSrc.pop();
    taskDst.pop();
  }

  return Status::Success;
}

// Dump everything to recycle chest.
// Player is src, recycle chest is dst.
Status DumpItems(BehaviourClient& c) {
  std::cout << GetTime() << "Trying to dump items to recycle chest..." << std::endl;
  Artist& artist = static_cast<Artist&>(c);
  std::shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  std::vector<Position> chestPositions = artist.board.Get<std::vector<Position>>("chest:recycle");

  for (auto chest : chestPositions) {
    if(FindPathAndMove(c, chest,  2, 2, 0, 6, 2, 2,  2, 2, 0, 3, 2, 2) == Status::Failure) continue;
    std::cout << GetTime() << "dumping items..." << std::endl;
    if (OpenContainer(c, chest) == Status::Failure) {
      std::cout << GetTime() << "Open Chest " << chest << " Error" << std::endl;
      continue;
    }

    std::queue<short> slotSrc, slotDst;
    short containerId, firstPlayerIndex;

    // Find possible swaps
    containerId = inventory_manager->GetFirstOpenedWindowId();
    if (containerId == -1) continue;

    std::shared_ptr<Window> container = inventory_manager->GetWindow(containerId);
    firstPlayerIndex = container->GetFirstPlayerInventorySlot();

    const std::map<short, Slot>& slots = container->GetSlots();

    for (auto it = slots.begin(); it != slots.end(); ++it) {
      if (it->first >= 0 && it->first < firstPlayerIndex && it->second.IsEmptySlot()) {
        slotDst.push(it->first);
      } else if (it->first >= firstPlayerIndex && !it->second.IsEmptySlot()) {
        std::string itemName = AssetsManager::getInstance().Items().at(it->second.GetItemID())->GetName();
        if (itemName == "minecraft:cooked_beef") continue;
        if (itemName.find("_pickaxe") != std::string::npos) continue;
        if (itemName.find("_axe") != std::string::npos) continue;
        if (itemName.find("_shovel") != std::string::npos) continue;
        if (itemName.find("shears") != std::string::npos) continue;

        slotSrc.push(it->first);
      }
    }

    while (!slotSrc.empty() && !slotDst.empty()) {
      std::cout << GetTime() << "Swap " << slotSrc.front() << " and " << slotDst.front() << std::endl;
      if (SwapItemsInContainer(c, containerId, slotSrc.front(), slotDst.front()) == Status::Failure) {
        CloseContainer(c, containerId);
        std::cout << GetTime() << "Error when trying to dump items to chest..." << std::endl;
      }
      slotSrc.pop();
      slotDst.pop();
    }

    // Close the chest
    CloseContainer(c, containerId);
    if (slotSrc.empty()) break;
  }
  std::cout << GetTime() << "Finish dumping items..." << std::endl;

  Status s = checkInventoryAllClear(c);
  if (s == Status::Failure) {
    std::cout << GetTime() << "Early stop due to recycle chest full..." << std::endl;
    MessageOutput("Early stop due to recycle chest full...", &artist);
    return Status::Failure;
  }

  return Status::Success;
}

Status TaskPrioritize(BehaviourClient& c) {
  Artist& artist = static_cast<Artist&>(c);
  std::string algo = artist.board.Get<std::string>("prioritize");
  artist.board.Set("Task.prioritized", true);

  if (algo == "bfs") {
    SimpleBFS(c);
  } else if (algo == "dfs") {
    SimpleDFS(c);
  } else if (algo == "slice_dfs") {
    SliceDFS(c);
  } else if (algo == "slice_dfs_neighbor") {
    SliceDFSNeighbor(c);
  } else {
    std::cout << GetTime() << "Get unrecognized prioritize method: " << algo << std::endl;
    return Status::Failure;
  }

  return Status::Success;
}

Status CollectAllMaterial(BehaviourClient& c) {
  std::cout << GetTime() << "Trying to collect material..." << std::endl;
  Artist& artist = static_cast<Artist&>(c);
  std::map<std::string, int, MaterialCompare> itemCounter = artist.board.Get<std::map<std::string, int, MaterialCompare>>("itemCounter");

  for (auto item : itemCounter) {
    Status s = CollectSingleMaterial(c, item.first, item.second);
    if (s == Status::Failure) {
      std::cout << "Early stop due to collect material fail..." << std::endl;
      MessageOutput("Early stop due to collect material fail...", &artist);
      return Status::Failure;
    }
  }
  return Status::Success;
}

Status CollectSingleMaterial(BehaviourClient& c, std::string itemName, int needed) {
  std::cout << GetTime() << "Collecting " << itemName << " for " << needed << std::endl;
  
  Artist& artist = static_cast<Artist&>(c);
  std::shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  std::vector<Position> availableChests = artist.board.Get<std::vector<Position>>("chest:"+itemName);

  bool get_all_material = false;
  int remain_empty_slot = -1;
  for (auto chest : availableChests) {
    std::cout << GetTime() << "========== CHEST ==========" << std::endl;
    SortInventory(c);
    Status moveResult = FindPathAndMove(c, chest,  2, 2, 2, 4, 2, 2,  0, 0, -1, 0, 0, 0);
    if (moveResult == Status::Failure) {
      std::cout << GetTime() << "Go to chest fail..." << std::endl;
      continue;
    }
    if (OpenContainer(c, chest) == Status::Failure) {
      std::cout << GetTime() << "Interact with chest fail..." << std::endl;
      continue;
    }
    
    int _need = needed;
    std::queue<short> canTake, canPut;
    const short containerId = inventory_manager->GetFirstOpenedWindowId();
    std::shared_ptr<Window> container = inventory_manager->GetWindow(containerId);

    {
      std::vector<std::pair<short, Slot>> _canPut, _canTake;
      const short playerInvStart = container->GetFirstPlayerInventorySlot();

      for (auto slot : container->GetSlots()) {
        if (slot.first < playerInvStart && !slot.second.IsEmptySlot()) {
          _canTake.push_back(slot);
        } else if (slot.first >= playerInvStart && !slot.second.IsEmptySlot()) {
          std::string _name = AssetsManager::getInstance().GetItem(slot.second.GetItemID())->GetName();
          if (_name == itemName && slot.second.GetItemCount() < 64) _canPut.push_back(slot);
          if (_name == itemName && slot.second.GetItemCount() == 64) _need -= 64;
        } else if (slot.first >= playerInvStart && slot.second.IsEmptySlot()) {
          _canPut.push_back(slot);
        }
      }

      sort(_canPut.begin(), _canPut.end(), [](const std::pair<const short, Slot>& a, const std::pair<const short, Slot>& b) {
        return a.second.GetItemCount() > b.second.GetItemCount();
      });
      for (auto slot : _canPut) {
        canPut.push(slot.first);
      }

      sort(_canTake.begin(), _canTake.end(), [](const std::pair<const short, Slot>& a, const std::pair<const short, Slot>& b) {
        return a.second.GetItemCount() > b.second.GetItemCount();
      });
      for (auto slot : _canTake) {
        canTake.push(slot.first);
      }
    }

    while (!canTake.empty() && !canPut.empty() && !get_all_material) {
      std::cout << GetTime() << "Swap from ID " << canTake.front() << " to ID " << canPut.front() << std::endl;
      Status swapResult = SwapItemsInContainer(c, containerId, canTake.front(), canPut.front());
      if (swapResult == Status::Failure){  // if failed, wait for a while and retry
        Utilities::SleepFor(std::chrono::milliseconds(500));
        std::cout << GetTime() << "Take " << itemName << " Failed" << std::endl;
        continue;
      }
      int took_amount = container->GetSlot(canPut.front()).GetItemCount();
      std::cout << GetTime() << "Take " << itemName << " for " << took_amount << std::endl;
      _need -= took_amount;
      canPut.pop();
      canTake.pop();

      if (_need < 1) get_all_material = true;
    }

    std::cout << GetTime() << "========== LIST ==========" << std::endl;
    {
      const short playerInvStart = container->GetFirstPlayerInventorySlot();
      remain_empty_slot = 0;
      for (auto p : container->GetSlots()){
        if (p.first >= playerInvStart) {
          if (p.second.IsEmptySlot()) {
            remain_empty_slot++;
            std::cout << GetTime() << "Slot" << p.first << ": " << std::endl;
          } else {
            std::cout << GetTime() << "Slot " << p.first << ": " 
              << AssetsManager::getInstance().Items().at(p.second.GetItemID())->GetName()
              << " x " << static_cast<int>(p.second.GetItemCount()) << std::endl;
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
      MessageOutput(itemName+" might not enough...", &artist);
    }
    return Status::Failure;
  }

  return Status::Success;
}

Status TaskExecutor(BehaviourClient& c) {
  std::cout << GetTime() << "Execute task in queue..." << std::endl;
  Artist& artist = static_cast<Artist&>(c);
  std::queue<Position> qTaskPosition = artist.board.Get<std::queue<Position>>("qTaskPosition");
  std::queue<std::string> qTaskType = artist.board.Get<std::queue<std::string>>("qTaskType");
  std::queue<std::string> qTaskName = artist.board.Get<std::queue<std::string>>("qTaskName");
  int retry_times = artist.board.Get<int>("retry");
  std::vector<Position> offsets {Position(1, 0, 0), Position(-1, 0, 0), Position(0, 0, 1), Position(0, 0, -1),
                            Position(2, 0, 0), Position(-2, 0, 0), Position(0, 0, 2), Position(0, 0, -2)};

  if (!qTaskPosition.empty() && !qTaskType.empty() && !qTaskName.empty()) {
    std::cout << GetTime() << "Remain " << qTaskPosition.size() << " tasks..." << std::endl;
    Position taskPos = qTaskPosition.front(); qTaskPosition.pop();
    std::string taskType = qTaskType.front(); qTaskType.pop();
    std::string blockName = qTaskName.front(); qTaskName.pop();
    for (int i = 0; i < retry_times; i++) {
      if (GetItemAmount(c, blockName) == 0 && taskType == "Place") {
        // Item not enough
        // Directly clear all tasks
        std::cout << GetTime() << "Item " << blockName << " not enough, return." << std::endl;
        qTaskPosition = std::queue<Position>();
        qTaskType = std::queue<std::string>();
        qTaskName = std::queue<std::string>();
        break;
      }
      Status exec_result = ExecuteTask(c, taskType, taskPos, blockName);
      if (exec_result == Status::Success) break;
      else {
        auto nextPos = taskPos+offsets[i%offsets.size()];
        const Botcraft::Blockstate* block = c.GetWorld()->GetBlock(nextPos);
        if(block != nullptr && !block->IsAir()){  // simple detect
          std::cout << GetTime() << "Task fail, move to another position and try again (" << i << ")..." << std::endl;
          FindPathAndMove(c, nextPos,  0, 0, 3, 3, 0, 0,  -1, -1, -1, -1, -1, -1);
        }
      }
    }
    
    artist.board.Set("qTaskPosition", qTaskPosition);
    artist.board.Set("qTaskType", qTaskType);
    artist.board.Set("qTaskName", qTaskName);

    // Still has task in queue, return fail.
    return Status::Failure;
  } else {
    artist.board.Set("Task.prioritized", false);

    // All tasks resolved, return success.
    return Status::Success;
  }
}

Status ExecuteTask(BehaviourClient& c, std::string action, Position blockPos, std::string blockName) {
  std::cout << GetTime() << "Task:" << std::setw(5) << action <<
                      ", Block Name:" << std::setw(32) << blockName <<
                      ", Position:" << blockPos << std::endl;
  
  if(FindPathAndMove(c, blockPos,  3, 3, 3, 3, 3, 3,  0, 0, 0, 2, 0, 0) == Status::Failure){
    std::cout << GetTime() << "Move Error" << std::endl;
    return Status::Failure;
  }
  std::string bn = GetWorldBlock(c, blockPos);
  if (action == "Dig") {
    if (bn == "minecraft:air") return Status::Success;
    else return Dig(c, blockPos, true);
  } else if (action == "Place") {
    if (bn == "minecraft:air") return PlaceBlock(c, blockName, blockPos, std::nullopt, true, true);
    else if (bn != blockName) {
      Dig(c, blockPos, true);
      return Status::Failure;
    } else return Status::Success;
  }

  std::cout << GetTime() << "Unknown task in ExecuteNextTask..." << std::endl;
  return Status::Failure;
}

Status FindPathAndMove(BehaviourClient&c, Position pos, 
    int x_tol_pos, int x_tol_neg, int y_tol_pos, int y_tol_neg, int z_tol_pos, int z_tol_neg, 
    int excl_x_pos, int excl_x_neg, int excl_y_pos, int excl_y_neg, int excl_z_pos, int excl_z_neg) {
  try{
    pf::Position to{pos.x, pos.y, pos.z};
    if(excl_x_pos >= 0 || excl_x_neg >= 0 || excl_y_pos >= 0 || excl_y_neg >= 0 || excl_z_pos >= 0 || excl_z_neg >= 0){
      using RGoal = pf::goal::RangeGoal<pf::Position>;
      using EGoal = pf::goal::ExclusiveGoal<RGoal>;
      using CGoal = pf::goal::CombineGoal<RGoal, EGoal>;
      CGoal goal(
        RGoal(to, x_tol_pos, x_tol_neg, y_tol_pos, y_tol_neg, z_tol_pos, z_tol_neg), 
        EGoal(RGoal(to, excl_x_pos, excl_x_neg, excl_y_pos, excl_y_neg, excl_z_pos, excl_z_neg)));
      return FindPathAndMoveImpl(c, pos, goal);
    }else{
      pf::goal::RangeGoal<pf::Position> goal(to, x_tol_pos, x_tol_neg, y_tol_pos, y_tol_neg, z_tol_pos, z_tol_neg);
      return FindPathAndMoveImpl(c, pos, goal);
    }
  }catch(const std::exception &e){
    std::cerr << "Move Fatal Error" << std::endl;
    std::cerr << e.what() << std::endl;
    return Status::Failure;
  }
}

Status FindPathAndMoveImpl(BehaviourClient&c, Position pos, pf::goal::GoalBase<pf::Position> &goal) {
  Artist& artist = static_cast<Artist&>(c);
  auto finder = artist.finder;

  auto getFromPosition = [&]() -> pf::Position {
    pf::Position from;
    // get player location
    std::shared_ptr<LocalPlayer> local_player = c.GetEntityManager()->GetLocalPlayer();
    auto player_pos = local_player->GetPosition();
    from.x = static_cast<int>(floor(player_pos.x));
    from.y = static_cast<int>(floor(player_pos.y + 0.25)) - 1;
    from.z = static_cast<int>(floor(player_pos.z));
    return from;
  };
  
  pf::Position to{pos.x, pos.y, pos.z};
  bool r = false;
  for(int i = 0; i < 2; ++i){
    pf::Position from = getFromPosition();
    std::cout << GetTime() << "Find a path from " << from << " to " << to << "\n";
    // find path and go
    r = finder.findPathAndGo(from, goal, 15000);
    if(r) break;
    from = getFromPosition();  // get the latest position
    std::cout << GetTime() << "Failed, retry after 5 seconds..." << std::endl;
    Utilities::SleepFor(std::chrono::seconds(3));  // delay 3 seconds
  }

  if (!r) {
    std::cout << GetTime() << "Bot get stuck, try to teleport..." << std::endl;
    Utilities::SleepFor(std::chrono::seconds(5));  // delay 5 seconds
    std::string homeCommand = artist.board.Get<std::string>("home", "tp @p 0 0 0");
    std::cout << GetTime() << "Send TP command..." << std::endl;
    c.SendChatCommand(homeCommand);
    std::cout << GetTime() << "Wait for TP success..." << std::endl;
    
    // wait for 10 seconds
    if(static_cast<Artist&>(c).waitTP(std::chrono::seconds(10)) == false){  // false
      std::cout << GetTime() << "TP Failed..." << std::endl;
      return Status::Failure;
    }
    std::cout << GetTime() << "TP Success!!!" << std::endl;

    std::cout << GetTime() << "World loading..." << std::endl;
    WaitServerLoad(c);  // always return true
    std::cout << GetTime() << "Finish world loading" << std::endl;

    // update player's new position
    pf::Position from = getFromPosition();
    std::cout << GetTime() << "Find a path from " << from << " to " << to << "\n";
    r = finder.findPathAndGo(from, goal, 15000);
  }
  
  return (r ? Status::Success : Status::Failure);
}

/*
If everything is correct, return Success, otherwise return Failure.
*/
Status checkCompletion(BehaviourClient& c) {
  Artist& artist = static_cast<Artist&>(c);
  std::shared_ptr<World> world = c.GetWorld();
  Position anchor = artist.board.Get<Position>("anchor");

  Position target_pos, world_pos;

  std::vector<std::vector<std::vector<bool>>> mapMemory = artist.board.Get<std::vector<std::vector<std::vector<bool>>>>("map_memory");

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Position& start = artist.board.Get<Position>("Structure.start");
  const Position& end = artist.board.Get<Position>("Structure.end");
  const std::vector<std::vector<std::vector<short>>>& target = artist.board.Get<std::vector<std::vector<std::vector<short>>>>("Structure.target");
  const std::map<short, std::string>& palette = artist.board.Get<std::map<short, std::string>>("Structure.palette");

  Status isComplete = Status::Success;
  int workers = artist.board.Get<int>("workerNum", 1);
  int col = artist.board.Get<int>("workCol", 0);

  for (int x = start.x; x <= end.x; x++) {
    if ((x-start.x)%workers != col) continue;

    world_pos.x = x;
    target_pos.x = x - start.x;
    for (int y = start.y; y <= end.y; y++) {
      world_pos.y = y;
      target_pos.y = y - start.y;
      for (int z = start.z; z <= end.z; z++) {
        world_pos.z = z;
        target_pos.z = z - start.z;

        const short target_id = target[target_pos.x][target_pos.y][target_pos.z];
        std::string target_name = palette.at(target_id);
        
        std::string block_name = "minecraft:air";
        const Blockstate* block = world->GetBlock(world_pos);
        if (!world->IsLoaded(world_pos)) {
          continue;
        } else if (block) {
          block_name = block->GetName();
        }

        if (block_name == "minecraft:air" && target_name == "minecraft:air") {
          // continue if it is a air block
          continue;
        } else if (block_name == "minecraft:air" && target_name != "minecraft:air") {
          // Found air in real world, but it should be something else
          isComplete = Status::Failure;
          mapMemory[target_pos.x][target_pos.y][target_pos.z] = false;
        } else if (block_name != "minecraft:air" && target_name == "minecraft:air") {
          // Found something else, but it should be air.
          isComplete = Status::Failure;
          mapMemory[target_pos.x][target_pos.y][target_pos.z] = false;
        } else if (block_name != target_name) {
          // The name of block not match.
          isComplete = Status::Failure;
          mapMemory[target_pos.x][target_pos.y][target_pos.z] = false;
        }
      }
    }
  }

  artist.board.Set("map_memory", mapMemory);
  return isComplete;
}

Status CheckCompletion(BehaviourClient& c) {
  Artist& artist = static_cast<Artist&>(c);
  std::shared_ptr<World> world = c.GetWorld();
  Position anchor = artist.board.Get<Position>("anchor");

  Position target_pos, world_pos;

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Position& start = artist.board.Get<Position>("Structure.start");
  const Position& end = artist.board.Get<Position>("Structure.end");
  const Position size = end - start + Position(1, 1, 1);
  const std::vector<std::vector<std::vector<short>>>& target = artist.board.Get<std::vector<std::vector<std::vector<short>>>>("Structure.target");
  const std::map<short, std::string>& palette = artist.board.Get<std::map<short, std::string>>("Structure.palette");

  std::vector<Position> checkpoints {Position(size.x*0.3, 0, size.z*0.3), Position(size.x*0.6, 0, size.z*0.3), 
                                Position(size.x*0.3, 0, size.z*0.6), Position(size.x*0.6, 0, size.z*0.6)};

  // initialize map recorder
  // default value will set to true, if the block is incorrect will set to false
  std::vector<std::vector<std::vector<bool>>> mapMemory(size.x, std::vector(size.y, std::vector(size.z, true)));
  artist.board.Set("map_memory", mapMemory);

  const bool log_details = false;
  const bool log_errors = true;
  const bool full_check = true;

  Status isComplete = Status::Success;
  for (auto cp : checkpoints) {
    std::cout << GetTime() << "Check checkpoint..." << std::endl;
    Status moveResult = FindPathAndMove(c, anchor+cp,  0, 0, 5, 5, 0, 0,  -1, -1, -1, -1, -1, -1);
    if (moveResult == Status::Failure) {
      std::cout << GetTime() << "Move to checkpoint fail..." << std::endl;
    }
    if (checkCompletion(c) == Status::Failure) isComplete = Status::Failure;
  }

  // update xCheck
  mapMemory = artist.board.Get<std::vector<std::vector<std::vector<bool>>>>("map_memory");
  std::vector<bool> xCheck = std::vector(size.x, false);

  for (int x = 0; x < size.x; x++) {
    bool isAllDone = true;
    for (int y = 0; y < size.y; y++) {
      for (int z = 0; z < size.z; z++) {
        if (!mapMemory[x][y][z]) isAllDone = false;
      }
    }

    xCheck[x] = isAllDone;
  }

  artist.board.Set("SliceDFS.xCheck", xCheck);

  return isComplete;
}

Status WarnConsole(BehaviourClient& c, const std::string& msg) {
  std::cout << GetTime() << "[" << c.GetNetworkManager()->GetMyName() << "]: " << msg << std::endl;
  return Status::Success;
}

Status LoadNBT(BehaviourClient& c) {
  NBT::Value loaded_file;
  Artist& artist = static_cast<Artist&>(c);
  Position offset = artist.board.Get<Position>("anchor");
  std::string temp_block = artist.board.Get<std::string>("tempblock");
  std::string nbt_path = artist.board.Get<std::string>("nbt");

  try {
    std::ifstream infile(nbt_path, std::ios::binary);
    infile.unsetf(std::ios::skipws);
    infile >> loaded_file;
    infile.close();
  } catch (const std::exception& e) {
    std::cout << GetTime() << "Error loading NBT file " << e.what() << std::endl;
    return Status::Failure;
  }

  std::map<short, std::string> palette;
  palette[-1] = "minecraft:air";
  short id_temp_block = -1;
  std::map<short, int> num_blocks_used;

  if (!loaded_file.contains("palette") || !loaded_file["palette"].is_list_of<NBT::TagCompound>()) {
    std::cout << GetTime() << "Error loading NBT file, no palette TagCompound found" << std::endl;
    return Status::Failure;
  }

  const std::vector<NBT::TagCompound>& palette_list = loaded_file["palette"].as_list_of<NBT::TagCompound>();
  for (int i = 0; i < palette_list.size(); ++i) {
    const std::string& block_name = palette_list[i]["Name"].get<std::string>();
    palette[i] = block_name;
    num_blocks_used[i] = 0;
    if (block_name == temp_block) id_temp_block = i;
  }

  Position min(std::numeric_limits<int>().max(), std::numeric_limits<int>().max(), std::numeric_limits<int>().max());
  Position max(std::numeric_limits<int>().min(), std::numeric_limits<int>().min(), std::numeric_limits<int>().min());

  if (!loaded_file.contains("blocks") || !loaded_file["blocks"].is_list_of<NBT::TagCompound>()) {
    std::cout << GetTime() << "Error loading NBT file, no blocks TagCompound found" << std::endl;
    return Status::Failure;
  }

  const std::vector<NBT::TagCompound>& block_tag = loaded_file["blocks"].as_list_of<NBT::TagCompound>();
  for (const auto& c : block_tag) {
    const std::vector<int>& pos_list = c["pos"].as_list_of<int>();
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

  Position size = max - min + Position(1, 1, 1);
  Position start = offset;
  Position end = offset + size - Position(1, 1, 1);

  std::cout << GetTime() << "Start: " << start << " | " << "End: " << end << std::endl;

  // Fill the target area with air (-1)
  std::vector<std::vector<std::vector<short>>> target(size.x, std::vector<std::vector<short>>(size.y, std::vector<short>(size.z, -1)));

  // Read all block to place
  for (const auto& c : block_tag) {
    const int state = c["state"].get<int>();
    const std::vector<int>& pos_list = c["pos"].as_list_of<int>();
    const int x = pos_list[0];
    const int y = pos_list[1];
    const int z = pos_list[2];

    target[x - min.x][y - min.y][z - min.z] = state;
    num_blocks_used[state] += 1;
  }

  if (id_temp_block == -1) {
    std::cout << GetTime() << "Can't find the given temp block " << temp_block << " in the palette" << std::endl;
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

    std::cout << GetTime() << "Removed the bottom " << removed_layers << " layer" << (removed_layers > 1 ? "s" : "") << std::endl;
  }

  std::cout << GetTime() << "Total size: " << size << std::endl;

  std::stringstream needed;
  needed << "Block needed:\n";
  for (auto it = num_blocks_used.begin(); it != num_blocks_used.end(); ++it) {
    needed << std::setw(35) << palette[it->first] << "----" << it->second << "\n";
  }
  std::cout << GetTime() << needed.rdbuf() << std::endl;

  // Check if some block can't be placed (flying blocks)
  std::stringstream flyings;
  flyings << "Flying blocks, you might have to place them yourself:\n";
  Position target_pos;

  const std::vector<Position> neighbour_offsets(
      {Position(0, 1, 0), Position(0, -1, 0), Position(0, 0, 1),
       Position(0, 0, -1), Position(1, 0, 0), Position(-1, 0, 0)});

  for (int x = 0; x < size.x; x++) {
    target_pos.x = x;
    // If this block is on the floor, it's ok
    for (int y = 1; y < size.y; y++) {
      target_pos.y = y;
      for (int z = 0; z < size.z; z++) {
        target_pos.z = z;

        const short target_id = target[target_pos.x][target_pos.y][target_pos.z];

        if (target_id != -1) {
          // Check all target neighbours
          bool has_neighbour = false;
          for (int i = 0; i < neighbour_offsets.size(); i++) {
            const Position neighbour_pos = target_pos + neighbour_offsets[i];

            bool x_constrain = neighbour_pos.x >= 0 && neighbour_pos.x < size.x;
            bool y_constrain = neighbour_pos.y >= 0 && neighbour_pos.y < size.y;
            bool z_constrain = neighbour_pos.z >= 0 && neighbour_pos.z < size.z;
            if (x_constrain && y_constrain && z_constrain &&
                target[neighbour_pos.x][neighbour_pos.y][neighbour_pos.z] != -1) {
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

  artist.board.Set("Structure.start", start);
  artist.board.Set("Structure.end", end);
  artist.board.Set("Structure.target", target);
  artist.board.Set("Structure.palette", palette);
  artist.board.Set("Structure.loaded", true);

  return Status::Success;
}

// Deprecated, config will load in Artist constructor
Status LoadConfig(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  const std::string &configPath = blackboard.Get<std::string>("configPath");
  std::ifstream file(configPath, std::ios::in);

  if (!file.is_open()) {
    std::cerr << GetTime() << "Unable to open file: " + configPath << std::endl;
    return Status::Failure;
  }

  std::string line;
  while (std::getline(file, line)) {
    // if line start with '#' or is empty, skip
    if (line.empty() || line[0] == '#') continue;

    std::istringstream iss(line);
    std::string key, value;
    std::getline(iss, key, '=') && std::getline(iss, value);
    if (key == "anchor") {
      Position anchor = ParsePositionString(value);
      blackboard.Set("anchor", anchor);
    } else if (key == "nbt") {
      blackboard.Set("nbt", value);
    } else if (key == "tempblock") {
      blackboard.Set("tempblock", value);
    } else if (key == "prioritize") {
      blackboard.Set("prioritize", value);
    } else if (key == "home") {
      std::cout << "TP Home command: " << value << std::endl;
      blackboard.Set("home", value);
    } else if (key == "retry") {
      blackboard.Set("retry", stoi(value));
    } else if (key == "neighbor") {
      blackboard.Set("neighbor", value == "true");
    } else {
      std::vector<Position> posVec;
      std::istringstream _iss(value);
      std::string posGroup;
      while (std::getline(_iss, posGroup, ';')) {
        Position chestPos = ParsePositionString(posGroup);
        posVec.push_back(chestPos);
      }
      blackboard.Set("chest:" + key, posVec);
    }
  }

  file.close();
  blackboard.Set("Config.loaded", true);
  return Status::Success;
}

Status EatUntilFull(BehaviourClient& c, std::string food) {
  while (IsHungry(c, 20) == Status::Success) {
    Status r = Eat(c, food, true);
    if (r == Status::Failure) return Status::Failure;
  }

  return Status::Success;
}