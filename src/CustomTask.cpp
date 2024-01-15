#include "CustomTask.hpp"
#include "Algorithm.hpp"
#include "PathFinding.hpp"
#include "Utils.hpp"
#include "Artist.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Network/NetworkManager.hpp"
#include "botcraft/Utilities/MiscUtilities.hpp"
#include "botcraft/Utilities/SleepUtilities.hpp"
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

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;

Position parsePostionString(string posStr) {
  vector<int> integers;
  istringstream iss(posStr);
  string token;

  while (getline(iss, token, ',')) {
    try {
      int num = stoi(token);
      integers.push_back(num);
    } catch (const exception& e) {
      cerr << GetTime() << "Invalid position: " << token << endl;
    }
  }
  Position pos(integers);

  return pos;
}

Status WaitServerLoad(BehaviourClient& c) {
  shared_ptr<LocalPlayer> local_player = c.GetEntityManager()->GetLocalPlayer();
  Utilities::WaitForCondition([&]() {
    return local_player->GetPosition().y < 1000;
  }, 10000);

  return Status::Success;
}

Status GetFood(BehaviourClient& c, const string& food_name) {
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();

  // Sort the chest and make sure the first slot in hotbar is empty
  // Food will place in this slot
  SwapItemsInContainer(c, Window::PLAYER_INVENTORY_INDEX,
                          Window::INVENTORY_HOTBAR_START, 
                          Window::INVENTORY_OFFHAND_INDEX);

  const vector<Position>& chests = c.GetBlackboard().Get<vector<Position>>("chest:" + food_name);

  vector<size_t> chests_indices(chests.size());
  for (size_t i = 0; i < chests.size(); ++i) {
    chests_indices[i] = i;
  }

  short container_id;
  bool item_taken = false;

  for (size_t index = 0; index < chests.size(); ++index) {
    const size_t i = chests_indices[index];
    // If we can't open this chest for a reason
    FindPathAndMove(c, chests[i],  1, 1, 1, 1, 1, 1,  0, 0, 0, 0, 0, 0);
    if (OpenContainer(c, chests[i]) == Status::Failure) continue;

    short player_dst = -1;
    while (true) {
      vector<short> slots_src;
      container_id = inventory_manager->GetFirstOpenedWindowId();
      if (container_id == -1) continue;

      const shared_ptr<Window> container = inventory_manager->GetWindow(container_id);

      const short playerFirstSlot = container->GetFirstPlayerInventorySlot();
      player_dst = playerFirstSlot + 9 * 3;

      const map<short, Slot>& slots = container->GetSlots();

      slots_src.reserve(slots.size());

      for (auto it = slots.begin(); it != slots.end(); ++it) {
        // Chest is src
        if (it->first >= 0 && it->first < playerFirstSlot && !it->second.IsEmptySlot() &&
            AssetsManager::getInstance().Items().at(it->second.GetItemID())->GetName() == food_name)
          slots_src.push_back(it->first);
      }

      if (slots_src.size() > 0) {
        cout << GetTime() << "Found food in chest." << endl;
        const int src_index = 0;
        // Try to swap the items
        if (SwapItemsInContainer(c, container_id, slots_src[src_index], player_dst) == Status::Success) {
          cout << GetTime() << "Get food success." << endl;
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
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  shared_ptr<Window> playerInv = inventory_manager->GetPlayerInventory();
  Status state = SortInventory(c);
  queue<short> taskSrc, taskDst;
  
  for (short i = Window::INVENTORY_STORAGE_START; i < Window::INVENTORY_HOTBAR_START+5; i++) {
    const Slot& slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) continue;
    string itemName = AssetsManager::getInstance().Items().at(slot.GetItemID())->GetName();
    if (itemName.find("_pickaxe") != string::npos) {
      // put pickaxe at slot 44
      taskSrc.push(i);
      taskDst.push(44);
      continue;
    }
    if (itemName.find("_axe") != string::npos) {
      // put axe at slot 43
      taskSrc.push(i);
      taskDst.push(43);
      continue;
    }
    if (itemName.find("_shovel") != string::npos) {
      // put shovel at slot 42
      taskSrc.push(i);
      taskDst.push(42);
      continue;
    }
    if (itemName.find("shears") != string::npos) {
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
  cout << GetTime() << "Trying to dump items to recycle chest..." << endl;
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  vector<Position> chestPositions = blackboard.Get<vector<Position>>("chest:recycle");

  for (auto chest : chestPositions) {
    if(FindPathAndMove(c, chest,  2, 2, 0, 6, 2, 2,  2, 2, 0, 3, 2, 2) == Status::Failure) continue;
    cout << GetTime() << "dumping items..." << endl;
    if (OpenContainer(c, chest) == Status::Failure) continue;

    queue<short> slotSrc, slotDst;
    short containerId, firstPlayerIndex;

    // Find possible swaps
    containerId = inventory_manager->GetFirstOpenedWindowId();
    if (containerId == -1) continue;

    shared_ptr<Window> container = inventory_manager->GetWindow(containerId);
    firstPlayerIndex = container->GetFirstPlayerInventorySlot();

    const map<short, Slot>& slots = container->GetSlots();

    for (auto it = slots.begin(); it != slots.end(); ++it) {
      if (it->first >= 0 && it->first < firstPlayerIndex && it->second.IsEmptySlot()) {
        slotDst.push(it->first);
      } else if (it->first >= firstPlayerIndex && !it->second.IsEmptySlot()) {
        string itemName = AssetsManager::getInstance().Items().at(it->second.GetItemID())->GetName();
        if (itemName == "minecraft:cooked_beef") continue;
        if (itemName.find("_pickaxe") != string::npos) continue;
        if (itemName.find("_axe") != string::npos) continue;
        if (itemName.find("_shovel") != string::npos) continue;
        if (itemName.find("shears") != string::npos) continue;

        slotSrc.push(it->first);
      }
    }

    while (!slotSrc.empty() && !slotDst.empty()) {
      if (SwapItemsInContainer(c, containerId, slotSrc.front(), slotDst.front()) == Status::Failure) {
        CloseContainer(c, containerId);
        cout << GetTime() << "Error when trying to dump items to chest..." << endl;
      }
      slotSrc.pop();
      slotDst.pop();
    }

    // Close the chest
    CloseContainer(c, containerId);
    if (slotSrc.empty()) break;
  }

  return Status::Success;
}

Status TaskPrioritize(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  string algo = blackboard.Get<string>("prioritize");
  blackboard.Set("Task.prioritized", true);

  if (algo == "bfs") {
    SimpleBFS(c);
  } else if (algo == "dfs") {
    SimpleDFS(c);
  } else if (algo == "slice_dfs") {
    SliceDFS(c);
  } else if (algo == "slice_dfs_neighbor") {
    SliceDFSNeighbor(c);
  } else {
    cout << GetTime() << "Get unrecognized prioritize method: " << algo << endl;
    return Status::Failure;
  }

  return Status::Success;
}

Status CollectAllMaterial(BehaviourClient& c) {
  cout << GetTime() << "Trying to collect material..." << endl;
  Blackboard& blackboard = c.GetBlackboard();
  // map<string, int, MaterialCompareOld> itemCounter = blackboard.Get<map<string, int, MaterialCompareOld>>("itemCounter");
  map<string, int, MaterialCompare> itemCounter = blackboard.Get<map<string, int, MaterialCompare>>("itemCounter");

  for (auto item : itemCounter) {
    CollectSingleMaterial(c, item.first, item.second);
  }
  return Status::Success;
}

Status CollectSingleMaterial(BehaviourClient& c, string itemName, int needed) {
  cout << GetTime() << "Collecting " << itemName << " for " << needed << endl;
  
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  vector<Position> availableChests = blackboard.Get<vector<Position>>("chest:"+itemName);

  bool get_all_material = false;
  for (auto chest : availableChests) {
    cout << GetTime() << "========== CHEST ==========" << endl;
    SortInventory(c);
    Status moveResult = FindPathAndMove(c, chest,  2, 2, 2, 2, 2, 2,  0, 0, -1, 0, 0, 0);
    if (moveResult == Status::Failure) {
      cout << GetTime() << "Go to chest fail..." << endl;
      continue;
    }
    if (OpenContainer(c, chest) == Status::Failure) {
      cout << GetTime() << "Interact with chest fail..." << endl;
      continue;
    }
    
    int _need = needed;
    queue<short> canTake, canPut;
    const short containerId = inventory_manager->GetFirstOpenedWindowId();
    shared_ptr<Window> container = inventory_manager->GetWindow(containerId);

    {
      vector<pair<short, Slot>> _canPut, _canTake;
      const short playerInvStart = container->GetFirstPlayerInventorySlot();

      for (auto slot : container->GetSlots()) {
        if (slot.first < playerInvStart && !slot.second.IsEmptySlot()) {
          _canTake.push_back(slot);
        } else if (slot.first >= playerInvStart && !slot.second.IsEmptySlot()) {
          string _name = AssetsManager::getInstance().GetItem(slot.second.GetItemID())->GetName();
          if (_name == itemName && slot.second.GetItemCount() < 64) _canPut.push_back(slot);
          if (_name == itemName && slot.second.GetItemCount() == 64) _need -= 64;
        } else if (slot.first >= playerInvStart && slot.second.IsEmptySlot()) {
          _canPut.push_back(slot);
        }
      }

      sort(_canPut.begin(), _canPut.end(), [](const pair<const short, Slot>& a, const pair<const short, Slot>& b) {
        return a.second.GetItemCount() > b.second.GetItemCount();
      });
      for (auto slot : _canPut) {
        canPut.push(slot.first);
      }

      sort(_canTake.begin(), _canTake.end(), [](const pair<const short, Slot>& a, const pair<const short, Slot>& b) {
        return a.second.GetItemCount() > b.second.GetItemCount();
      });
      for (auto slot : _canTake) {
        canTake.push(slot.first);
      }
    }

    while (!canTake.empty() && !canPut.empty() && !get_all_material) {
      cout << GetTime() << "Swap from ID " << canTake.front() << " to ID " << canPut.front() << endl;
      Status swapResult = SwapItemsInContainer(c, containerId, canTake.front(), canPut.front());
      if (swapResult == Status::Failure){  // if failed, wait for a while and retry
        Utilities::SleepFor(chrono::milliseconds(500));
        cout << GetTime() << "Take " << itemName << " Failed" << endl;
        continue;
      }
      int took_amount = container->GetSlot(canPut.front()).GetItemCount();
      cout << GetTime() << "Take " << itemName << " for " << took_amount << endl;
      _need -= took_amount;
      canPut.pop();
      canTake.pop();

      if (_need < 1) get_all_material = true;
    }

    cout << GetTime() << "========== LIST ==========" << endl;
    {
      const short playerInvStart = container->GetFirstPlayerInventorySlot();
      for (auto p : container->GetSlots()){
        if (p.first >= playerInvStart && !p.second.IsEmptySlot()){
          cout << GetTime() << "Slot " << p.first << ": " 
              << AssetsManager::getInstance().Items().at(p.second.GetItemID())->GetName()
              << " x " << static_cast<int>(p.second.GetItemCount()) << endl;
        }
      }
    }
    cout << GetTime() << "======= LIST CLOSE =======" << endl;

    CloseContainer(c, containerId);
    cout << GetTime() << "======= CHEST CLOSE =======" << endl;
    if (get_all_material) break;
  }

  if (!get_all_material) {
    cout << GetTime() << itemName << " might not enough..." << endl;
    Say(c, itemName+" might not enough...");
  }

  return Status::Success;
}

Status TaskExecutor(BehaviourClient& c) {
  cout << GetTime() << "Execute task in queue..." << endl;
  Blackboard& blackboard = c.GetBlackboard();
  queue<Position> qTaskPosition = blackboard.Get<queue<Position>>("qTaskPosition");
  queue<string> qTaskType = blackboard.Get<queue<string>>("qTaskType");
  queue<string> qTaskName = blackboard.Get<queue<string>>("qTaskName");
  int retry_times = blackboard.Get<int>("retry");
  vector<Position> offsets {Position(1, 0, 0), Position(-1, 0, 0), Position(0, 0, 1), Position(0, 0, -1),
                            Position(2, 0, 0), Position(-2, 0, 0), Position(0, 0, 2), Position(0, 0, -2)};

  if (!qTaskPosition.empty() && !qTaskType.empty() && !qTaskName.empty()) {
    cout << GetTime() << "Remain " << qTaskPosition.size() << " tasks..." << endl;
    Position taskPos = qTaskPosition.front(); qTaskPosition.pop();
    string taskType = qTaskType.front(); qTaskType.pop();
    string blockName = qTaskName.front(); qTaskName.pop();
    for (int i = 0; i < retry_times; i++) {
      if (GetItemAmount(c, blockName) == 0 && taskType == "Place") {
        // Item not enough
        // Directly clear all tasks
        cout << GetTime() << "Item " << blockName << " not enough, return." << endl;
        qTaskPosition = queue<Position>();
        qTaskType = queue<string>();
        qTaskName = queue<string>();
        break;
      }
      Status exec_result = ExecuteTask(c, taskType, taskPos, blockName);
      if (exec_result == Status::Success) break;
      else {
        auto nextPos = taskPos+offsets[i%offsets.size()];
        const Botcraft::Blockstate* block = c.GetWorld()->GetBlock(nextPos);
        if(block != nullptr && !block->IsAir()){  // simple detect
          cout << GetTime() << "Task fail, move to another position and try again (" << i << ")..." << endl;
          FindPathAndMove(c, nextPos,  0, 0, 3, 3, 0, 0,  -1, -1, -1, -1, -1, -1);
        }
      }
    }
    
    blackboard.Set("qTaskPosition", qTaskPosition);
    blackboard.Set("qTaskType", qTaskType);
    blackboard.Set("qTaskName", qTaskName);

    // Still has task in queue, return fail.
    return Status::Failure;
  } else {
    blackboard.Set("Task.prioritized", false);

    // All tasks resolved, return success.
    return Status::Success;
  }
}

Status ExecuteTask(BehaviourClient& c, string action, Position blockPos, string blockName) {
  cout << GetTime() << "Task:" << setw(5) << action <<
                      ", Block Name:" << setw(32) << blockName <<
                      ", Position:" << blockPos << endl;
  
  Blackboard& board = c.GetBlackboard();

  if(FindPathAndMove(c, blockPos,  3, 3, 3, 3, 3, 3,  0, 0, 0, 2, 0, 0) == Status::Failure){
    cout << GetTime() << "Move Error" << endl;
    return Status::Failure;
  }
  string bn = GetWorldBlock(c, blockPos);
  if (action == "Dig") {
    if (bn == "minecraft:air") return Status::Success;
    else return Dig(c, blockPos, true);
  } else if (action == "Place") {
    if (bn == "minecraft:air") return PlaceBlock(c, blockName, blockPos, nullopt, true, true);
    else if (bn != blockName) {
      Dig(c, blockPos, true);
      return Status::Failure;
    } else return Status::Success;
  }

  cout << GetTime() << "Unknown task in ExecuteNextTask..." << endl;
  return Status::Failure;
}

Status FindPathAndMove(BehaviourClient&c, Position pos, 
    int x_tol_pos, int x_tol_neg, int y_tol_pos, int y_tol_neg, int z_tol_pos, int z_tol_neg, 
    int excl_x_pos, int excl_x_neg, int excl_y_pos, int excl_y_neg, int excl_z_pos, int excl_z_neg) {
  pf::Position to{pos.x, pos.y, pos.z};
  std::unique_ptr<pf::goal::GoalBase<pf::Position>> goal;
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
}

Status FindPathAndMoveImpl(BehaviourClient&c, Position pos, pf::goal::GoalBase<pf::Position> &goal) {
  Blackboard& blackboard = c.GetBlackboard();
  auto finder = blackboard.Get<PathFinder>("pathFinder");

  // get player location
  pf::Position from, to{pos.x, pos.y, pos.z};
  shared_ptr<LocalPlayer> local_player = c.GetEntityManager()->GetLocalPlayer();
  auto player_pos = local_player->GetPosition();
  from.x = static_cast<int>(floor(player_pos.x));
  from.y = static_cast<int>(floor(player_pos.y)) - 1;
  from.z = static_cast<int>(floor(player_pos.z));
  
  std::cout << GetTime() << "Find a path from " << from << " to " << to << "\n";
  bool r = false;
  for(int i = 0; i < 2; ++i){
    r = finder.findPathAndGo(from, goal, 15000);
    if(r) break;
    cout << GetTime() << "Failed, retry after 5 seconds..." << endl;
    Utilities::SleepFor(chrono::seconds(5));  // delay 5 seconds
  }

  if (!r) {
    cout << GetTime() << "Bot get stuck, try to teleport..." << endl;
    Utilities::SleepFor(chrono::seconds(5));  // delay 5 seconds
    auto tp_future = static_cast<Artist&>(c).waitTP();
    string homeName = blackboard.Get<string>("home", "mapart");
    cout << GetTime() << "Send TP Command" << endl;
    c.SendChatCommand("homes " + homeName);
    cout << GetTime() << "Wait for TP..." << endl;
    
    // wait for 10 seconds
    if(tp_future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout){
      cout << GetTime() << "TP Failed" << endl;
      return Status::Failure;
    }
    cout << GetTime() << "TP Success" << endl;

    cout << GetTime() << "World loading..." << endl;
    WaitServerLoad(c);  // always return true
    cout << GetTime() << "Finish world loading" << endl;

    // update player's new position
    auto player_pos = c.GetEntityManager()->GetLocalPlayer()->GetPosition();
    from.x = static_cast<int>(floor(player_pos.x));
    from.y = static_cast<int>(floor(player_pos.y)) - 1;
    from.z = static_cast<int>(floor(player_pos.z));
    cout << GetTime() << "Find path" << endl;
    r = finder.findPathAndGo(from, goal, 15000);
  }
  
  return (r ? Status::Success : Status::Failure);
}

/*
If everything is correct, return Success, otherwise return Failure.
*/
Status checkCompletion(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();
  Position anchor = blackboard.Get<Position>("anchor");

  Position target_pos, world_pos;

  vector<vector<vector<bool>>> mapMemory = blackboard.Get<vector<vector<vector<bool>>>>("map_memory");

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");

  Status isComplete = Status::Success;
  int workers = blackboard.Get<int>("workerNum", 1);
  int col = blackboard.Get<int>("workCol", 0);

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
        string target_name = palette.at(target_id);
        
        string block_name = "minecraft:air";
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

  blackboard.Set("map_memory", mapMemory);
  return isComplete;
}

Status CheckCompletion(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();
  Position anchor = blackboard.Get<Position>("anchor");

  Position target_pos, world_pos;

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position size = end - start + Position(1, 1, 1);
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");

  vector<Position> checkpoints {Position(size.x*0.3, 0, size.z*0.3), Position(size.x*0.6, 0, size.z*0.3), 
                                Position(size.x*0.3, 0, size.z*0.6), Position(size.x*0.6, 0, size.z*0.6)};

  // initialize map recorder
  // default value will set to true, if the block is incorrect will set to false
  vector<vector<vector<bool>>> mapMemory(size.x, vector(size.y, vector(size.z, true)));
  blackboard.Set("map_memory", mapMemory);

  const bool log_details = false;
  const bool log_errors = true;
  const bool full_check = true;

  Status isComplete = Status::Success;
  for (auto cp : checkpoints) {
    cout << GetTime() << "Check checkpoint..." << endl;
    Status moveResult = FindPathAndMove(c, anchor+cp,  0, 0, 5, 5, 0, 0,  -1, -1, -1, -1, -1, -1);
    if (moveResult == Status::Failure) {
      cout << GetTime() << "Move to checkpoint fail..." << endl;
    }
    if (checkCompletion(c) == Status::Failure) isComplete = Status::Failure;
  }

  // update xCheck
  mapMemory = blackboard.Get<vector<vector<vector<bool>>>>("map_memory");
  vector<bool> xCheck = vector(size.x, false);

  for (int x = 0; x < size.x; x++) {
    bool isAllDone = true;
    for (int y = 0; y < size.y; y++) {
      for (int z = 0; z < size.z; z++) {
        if (!mapMemory[x][y][z]) isAllDone = false;
      }
    }

    xCheck[x] = isAllDone;
  }

  blackboard.Set("SliceDFS.xCheck", xCheck);

  return isComplete;
}

Status WarnConsole(BehaviourClient& c, const string& msg) {
  cout << GetTime() << "[" << c.GetNetworkManager()->GetMyName() << "]: " << msg << endl;
  return Status::Success;
}

Status LoadNBT(BehaviourClient& c) {
  NBT::Value loaded_file;
  Blackboard& blackboard = c.GetBlackboard();
  Position offset = blackboard.Get<Position>("anchor");
  string temp_block = blackboard.Get<string>("tempblock");
  string nbt_path = blackboard.Get<string>("nbt");

  try {
    ifstream infile(nbt_path, ios::binary);
    infile.unsetf(ios::skipws);
    infile >> loaded_file;
    infile.close();
  } catch (const exception& e) {
    cout << GetTime() << "Error loading NBT file " << e.what() << endl;
    return Status::Failure;
  }

  map<short, string> palette;
  palette[-1] = "minecraft:air";
  short id_temp_block = -1;
  map<short, int> num_blocks_used;

  if (!loaded_file.contains("palette") || !loaded_file["palette"].is_list_of<NBT::TagCompound>()) {
    cout << GetTime() << "Error loading NBT file, no palette TagCompound found" << endl;
    return Status::Failure;
  }

  const vector<NBT::TagCompound>& palette_list = loaded_file["palette"].as_list_of<NBT::TagCompound>();
  for (int i = 0; i < palette_list.size(); ++i) {
    const string& block_name = palette_list[i]["Name"].get<string>();
    palette[i] = block_name;
    num_blocks_used[i] = 0;
    if (block_name == temp_block) id_temp_block = i;
  }

  Position min(numeric_limits<int>().max(), numeric_limits<int>().max(), numeric_limits<int>().max());
  Position max(numeric_limits<int>().min(), numeric_limits<int>().min(), numeric_limits<int>().min());

  if (!loaded_file.contains("blocks") || !loaded_file["blocks"].is_list_of<NBT::TagCompound>()) {
    cout << GetTime() << "Error loading NBT file, no blocks TagCompound found" << endl;
    return Status::Failure;
  }

  const vector<NBT::TagCompound>& block_tag = loaded_file["blocks"].as_list_of<NBT::TagCompound>();
  for (const auto& c : block_tag) {
    const vector<int>& pos_list = c["pos"].as_list_of<int>();
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

  cout << GetTime() << "Start: " << start << " | " << "End: " << end << endl;

  // Fill the target area with air (-1)
  vector<vector<vector<short>>> target(size.x, vector<vector<short>>(size.y, vector<short>(size.z, -1)));

  // Read all block to place
  for (const auto& c : block_tag) {
    const int state = c["state"].get<int>();
    const vector<int>& pos_list = c["pos"].as_list_of<int>();
    const int x = pos_list[0];
    const int y = pos_list[1];
    const int z = pos_list[2];

    target[x - min.x][y - min.y][z - min.z] = state;
    num_blocks_used[state] += 1;
  }

  if (id_temp_block == -1) {
    cout << GetTime() << "Can't find the given temp block " << temp_block << " in the palette" << endl;
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

    cout << GetTime() << "Removed the bottom " << removed_layers << " layer" << (removed_layers > 1 ? "s" : "") << endl;
  }

  cout << GetTime() << "Total size: " << size << endl;

  stringstream needed;
  needed << "Block needed:\n";
  for (auto it = num_blocks_used.begin(); it != num_blocks_used.end(); ++it) {
    needed << setw(35) << palette[it->first] << "----" << it->second << "\n";
  }
  cout << GetTime() << needed.rdbuf() << endl;

  // Check if some block can't be placed (flying blocks)
  stringstream flyings;
  flyings << "Flying blocks, you might have to place them yourself:\n";
  Position target_pos;

  const vector<Position> neighbour_offsets(
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
  cout << GetTime() << flyings.rdbuf() << endl;

  blackboard.Set("Structure.start", start);
  blackboard.Set("Structure.end", end);
  blackboard.Set("Structure.target", target);
  blackboard.Set("Structure.palette", palette);
  blackboard.Set("Structure.loaded", true);

  return Status::Success;
}

Status LoadConfig(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  const string &configPath = blackboard.Get<string>("configPath");
  ifstream file(configPath, ios::in);

  if (!file.is_open()) {
    cerr << GetTime() << "Unable to open file: " + configPath << endl;
    return Status::Failure;
  }

  string line;
  while (getline(file, line)) {
    // if line start with '#' or is empty, skip
    if (line.empty() || line[0] == '#') continue;

    istringstream iss(line);
    string key, value;
    getline(iss, key, '=') && getline(iss, value);
    if (key == "anchor") {
      Position anchor = parsePostionString(value);
      blackboard.Set("anchor", anchor);
    } else if (key == "nbt") {
      blackboard.Set("nbt", value);
    } else if (key == "tempblock") {
      blackboard.Set("tempblock", value);
    } else if (key == "prioritize") {
      blackboard.Set("prioritize", value);
    } else if (key == "home") {
      cout << "Set Home Point: " << value << endl;
      blackboard.Set("home", value);
    } else if (key == "retry") {
      blackboard.Set("retry", stoi(value));
    } else if (key == "neighbor") {
      blackboard.Set("neighbor", value == "true");
    } else {
      vector<Position> posVec;
      istringstream _iss(value);
      string posGroup;
      while (getline(_iss, posGroup, ';')) {
        Position chestPos = parsePostionString(posGroup);
        posVec.push_back(chestPos);
      }
      blackboard.Set("chest:" + key, posVec);
    }
  }

  file.close();
  blackboard.Set("Config.loaded", true);
  return Status::Success;
}

Status EatUntilFull(BehaviourClient& c, string food) {
  while (IsHungry(c, 20) == Status::Success) {
    Status r = Eat(c, food, true);
    if (r == Status::Failure) return Status::Failure;
  }

  return Status::Success;
}