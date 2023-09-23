#include "CustomTask.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <map>

#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Network/NetworkManager.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "botcraft/Utilities/SleepUtilities.hpp"
#include "botcraft/Utilities/MiscUtilities.hpp"

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
      cerr << "Invalid position: " << token << endl;
    }
  }
  Position pos(integers);

  return pos;
}

Status WaitServerLoad(BehaviourClient& c) {
  shared_ptr<LocalPlayer> local_player = c.GetEntityManager()->GetLocalPlayer();
  Utilities::WaitForCondition([&]() {
        lock_guard<mutex> player_lock(local_player->GetMutex());
        return local_player->GetPosition().y < 1000;
      }, 10000);

  return Status::Success;
}

Status GetFood(BehaviourClient& c, const string& food_name) {
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);

  // Sort the chest and make sure the first slot in hotbar is empty
  // Food will place in this slot
  // SortInventory(c);
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
    if (OpenContainer(c, chests[i]) == Status::Failure) continue;

    short player_dst = -1;
    while (true) {
      vector<short> slots_src;
      {
        lock_guard<mutex> inventory_lock(inventory_manager->GetMutex());
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
      }

      if (slots_src.size() > 0) {
        LOG_INFO("Found food in chest.");
        const int src_index = 0;
        // Try to swap the items
        if (SwapItemsInContainer(c, container_id, slots_src[src_index], player_dst) == Status::Success) {
          LOG_INFO("Get food success.");
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
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Status state = SortInventory(c);
  queue<short> taskSrc, taskDst;
  {
    lock_guard<mutex> inventoryLock(inventory_manager->GetMutex());
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
  LOG_INFO("Trying to dump items to recycle chest...");
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  vector<Position> chestPositions = blackboard.Get<vector<Position>>("chest:recycle");

  for (auto chest : chestPositions) {
    if (OpenContainer(c, chest) == Status::Failure) continue;

    queue<short> slotSrc, slotDst;
    short containerId, firstPlayerIndex;

    // Find possible swaps
    {
      lock_guard<mutex> inventoryLock(inventory_manager->GetMutex());
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
    }

    while (!slotSrc.empty() && !slotDst.empty()) {
      if (SwapItemsInContainer(c, containerId, slotSrc.front(), slotDst.front()) == Status::Failure) {
        CloseContainer(c, containerId);
        LOG_WARNING("Error when trying to dump items to chest.");
      }
      slotSrc.pop();
      slotDst.pop();
    }

    // Close the chest
    CloseContainer(c, containerId);
    if (slotDst.empty()) break;
  }

  return Status::Success;
}

Status TaskPriortize(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  // shared_ptr<EntityManager> entityMgr = c.GetEntityManager();
  shared_ptr<World> world = c.GetWorld();

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position& anchor = blackboard.Get<Position>("anchor");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");

  const Position size = end - start + Position(1, 1, 1);
  vector<vector<vector<bool>>> visited(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));

  int slotCounter = 0;
  map<string, int> itemCounter;

  queue<Position> pending, qTaskPosition;
  queue<string> qTaskType, qTaskName;
  pending.push(Position(0, 0, 0));
  visited[0][0][0] = true;

  const vector<Position> neighbor_offsets({ Position(0, 1, 0), Position(0, -1, 0), 
                                            Position(0, 0, 1), Position(0, 0, -1),
                                            Position(1, 0, 0), Position(-1, 0, 0) });

  while (!pending.empty()) {
    Position currentPos = pending.front();
    pending.pop();
    
    const short current_target = target[currentPos.x][currentPos.y][currentPos.z];
    const string targetName = palette.at(current_target);

    // for blocks in pending queue, we need to check below condition:
    // 1. if this block is empty and not air, then place it. (push "Place" to qTaskType)
    // 2. if this block is not air, then check if it is the same block in nbt.
    // 2-1. skip this block if it is correct.
    // 2-2. remove this block if it is incorrect. (push "Dig" to qTaskType)
    // 3. check its neighbors.
    // 3-1 if this neighbor already visited, skip
    // 3-2 if not visited, push to pending and mark it as visited
    string block_name = "minecraft:air";
    {
      lock_guard<mutex> worldLock(world->GetMutex());
      const Block* block = world->GetBlock(currentPos+anchor);
      if (!block) {
        // it is a air block
        if (!world->IsLoaded(currentPos+anchor)) {
          GoTo(c, currentPos+anchor);
          block = world->GetBlock(currentPos+anchor);
          if (block) block_name = block->GetBlockstate()->GetName();
        }
      } else {
        block_name = block->GetBlockstate()->GetName();
      }
    }

    if (targetName != "minecraft:air" && current_target != -1 && block_name == "minecraft:air") {
      qTaskPosition.push(currentPos+anchor);
      qTaskType.push("Place");
      qTaskName.push(targetName);

      // maintain itemCounter and slotCounter
      if ((itemCounter[targetName]++) % 64 == 0) slotCounter++;
      if (slotCounter == 27) break;
    } else if (block_name != "minecraft:air" && targetName != block_name) {
      qTaskPosition.push(currentPos+anchor);
      qTaskType.push("Dig");
      qTaskName.push(targetName);
    }

    for (int i = 0; i < neighbor_offsets.size(); i++) {
      Position newPos = currentPos + neighbor_offsets[i];
      bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
      bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
      bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

      if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
        visited[newPos.x][newPos.y][newPos.z] = true;
        pending.push(newPos);
      }
    }
  }

  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);

  return Status::Success;
}

Status CollectAllMaterial(BehaviourClient& c) {
  LOG_INFO("Trying to collect material...");
  Blackboard& blackboard = c.GetBlackboard();
  map<string, int> itemCounter = blackboard.Get<map<string, int>>("itemCounter");

  for (auto item : itemCounter) {
    CollectSingleMaterial(c, item.first, item.second);
  }
  return Status::Success;
}

Status CollectSingleMaterial(BehaviourClient& c, string itemName, int needed) {
  LOG_INFO("Collecting " << itemName << " for " << needed);
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  vector<Position> availableChests = blackboard.Get<vector<Position>>("chest:"+itemName);

  for (auto chest : availableChests) {
    SortInventory(c);
    if (OpenContainer(c, chest) == Status::Failure) continue;
    
    queue<short> canTake, canPut;
    const short containerId = inventory_manager->GetFirstOpenedWindowId();
    shared_ptr<Window> container = inventory_manager->GetWindow(containerId);

    {
      lock_guard<mutex> lock(inventory_manager->GetMutex());
      vector<pair<short, Slot>> _canPut;
      const short playerInvStart = container->GetFirstPlayerInventorySlot();

      for (auto slot : container->GetSlots()) {
        if (slot.first < playerInvStart && !slot.second.IsEmptySlot()) {
          canTake.push(slot.first);
        } else if (slot.first >= playerInvStart && !slot.second.IsEmptySlot()) {
          string _name = AssetsManager::getInstance().GetItem(slot.second.GetItemID())->GetName();
          if (_name == itemName && slot.second.GetItemCount() < 64) _canPut.push_back(slot);
          if (_name == itemName && slot.second.GetItemCount() == 64) needed -= 64;
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
    }

    while (!canTake.empty() && !canPut.empty()) {
      SwapItemsInContainer(c, containerId, canTake.front(), canPut.front());
      lock_guard<mutex> lock(inventory_manager->GetMutex());
      needed -= container->GetSlot(canPut.front()).GetItemCount();
      canPut.pop();
      canTake.pop();

      if (needed < 1) break;
    }

    CloseContainer(c, containerId);
    if (needed < 1) break;
  }

  if (needed > 0) {
    LOG_WARNING(itemName << " might not enough...");
    Say(c, itemName+" might not enough...");
  }

  return Status::Success;
}

Status TaskExecutor(BehaviourClient& c) {
  LOG_INFO("Execute task in queue...");
  Blackboard& blackboard = c.GetBlackboard();
  queue<Position> qTaskPosition = blackboard.Get<queue<Position>>("qTaskPosition");
  queue<string> qTaskType = blackboard.Get<queue<string>>("qTaskType");
  queue<string> qTaskName = blackboard.Get<queue<string>>("qTaskName");
  map<string, int> itemCounter = blackboard.Get<map<string, int>>("itemCounter");

  while (!qTaskPosition.empty() && !qTaskType.empty() && !qTaskName.empty()) {
    Position taskPos = qTaskPosition.front();
    string taskType = qTaskType.front();
    string blockName = qTaskName.front();
    ExecuteTask(c, taskType, taskPos, blockName);
    qTaskPosition.pop();
    qTaskType.pop();
    qTaskName.pop();
  }

  return Status::Success;
}

Status ExecuteTask(BehaviourClient& c, string action, Position blockPos, string blockName) {
  LOG_INFO("Execute the task...");
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Blackboard& board = c.GetBlackboard();

  if (action == "Dig") {
    // GoTo(c, blockPos);
    return Dig(c, blockPos, true);
  } else if (action == "Place") {
    // GoTo(c, blockPos);
    return PlaceBlock(c, blockName, blockPos, nullopt, true, true);
  }

  LOG_WARNING("Unknown task in ExecuteNextTask");
  return Status::Failure;
}

// TODO: review code and optimize
Status CheckCompletion(BehaviourClient& c) {
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();
  Position anchor = blackboard.Get<Position>("anchor");

  Position target_pos, world_pos;

  int additional_blocks = 0;
  int wrong_blocks = 0;
  int missing_blocks = 0;

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");

  const bool log_details = false;
  const bool log_errors = true;
  const bool full_check = true;

  // Reset values for the next time
  //  blackboard.Set("CheckCompletion.log_details", false);
  //  blackboard.Set("CheckCompletion.log_errors", false);
  //  blackboard.Set("CheckCompletion.full_check", false);

  for (int x = start.x; x <= end.x; ++x) {
    world_pos.x = x;
    target_pos.x = x - anchor.x;
    for (int y = start.y; y <= end.y; ++y) {
      world_pos.y = y;
      target_pos.y = y - anchor.y;
      for (int z = start.z; z <= end.z; ++z) {
        world_pos.z = z;
        target_pos.z = z - anchor.z;

        const short target_id = target[target_pos.x][target_pos.y][target_pos.z];
        string target_name = palette.at(target_id);
        
        // TODO: fix blockstate issue
        string block_name = "minecraft:air";
        {
          lock_guard<mutex> worldLock(world->GetMutex());
          const Block* block = world->GetBlock(world_pos);
          if (!block) {
            if (!world->IsLoaded(world_pos)) {
              GoTo(c, target_pos);
              block = world->GetBlock(world_pos);
              if (block) block_name = block->GetBlockstate()->GetName();
            }
          } else {
            block_name = block->GetBlockstate()->GetName();
          }
        }

        if (block_name == "minecraft:air" && (target_id == -1 || target_name == "minecraft:air")) {
          // continue if it is a air block
          continue;
        } else if (block_name == "minecraft:air" && target_id != -1 && target_name != "minecraft:air") {
          // Found air in real world, but it should be something else
          LOG_INFO("Want: " << target_name << ", but got: " << block_name << " at: (" << target_pos.x << "," << target_pos.y << "," << target_pos.z << ")");
          missing_blocks++;
        } else if (block_name != "minecraft:air" && (target_id == -1 || target_name == "minecraft:air")) {
          // Found something else, but it should be air.
          wrong_blocks++;
        } else if (block_name != target_name) {
          // The name of block not match.
          wrong_blocks++;
        }
      }
    }
  }

  if (log_errors) {
    LOG_INFO("Wrong blocks: " << wrong_blocks);
    LOG_INFO("Missing blocks: " << missing_blocks);
    LOG_INFO("Additional blocks: " << additional_blocks);
  }

  return (missing_blocks + additional_blocks + wrong_blocks == 0)
             ? Status::Success
             : Status::Failure;
}

Status WarnConsole(BehaviourClient& c, const string& msg) {
  LOG_WARNING("[" << c.GetNetworkManager()->GetMyName() << "]: " << msg);
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
    LOG_ERROR("Error loading NBT file " << e.what());
    return Status::Failure;
  }

  map<short, string> palette;
  palette[-1] = "minecraft:air";
  short id_temp_block = -1;
  map<short, int> num_blocks_used;

  if (!loaded_file.contains("palette") || !loaded_file["palette"].is_list_of<NBT::TagCompound>()) {
    LOG_ERROR("Error loading NBT file, no palette TagCompound found");
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
    LOG_ERROR("Error loading NBT file, no blocks TagCompound found");
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

  LOG_INFO("Start: " << start << " | " << "End: " << end);

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
    LOG_WARNING("Can't find the given temp block " << temp_block << " in the palette");
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

    LOG_INFO("Removed the bottom " << removed_layers << " layer" << (removed_layers > 1 ? "s" : ""));
  }

  LOG_INFO("Total size: " << size);

  stringstream needed;
  needed << "Block needed:\n";
  for (auto it = num_blocks_used.begin(); it != num_blocks_used.end(); ++it) {
    needed << "\t" << palette[it->first] << "\t\t" << it->second << "\n";
  }
  LOG_INFO(needed.rdbuf());

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
  LOG_WARNING(flyings.rdbuf());

  blackboard.Set("Structure.start", start);
  blackboard.Set("Structure.end", end);
  blackboard.Set("Structure.target", target);
  blackboard.Set("Structure.palette", palette);
  blackboard.Set("Structure.loaded", true);

  return Status::Success;
}

Status LoadConfig(BehaviourClient& c) {
  ifstream file("config.txt");

  if (!file.is_open()) {
    cerr << "Unable to open file: config.txt" << endl;
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
      c.GetBlackboard().Set("anchor", anchor);
    } else if (key == "nbt") {
      c.GetBlackboard().Set("nbt", value);
    } else if (key == "tempblock") {
      c.GetBlackboard().Set("tempblock", value);
    } else {
      vector<Position> posVec;
      istringstream _iss(value);
      string posGroup;
      while (getline(_iss, posGroup, ';')) {
        Position chestPos = parsePostionString(posGroup);
        posVec.push_back(chestPos);
      }
      c.GetBlackboard().Set("chest:" + key, posVec);
    }
  }

  file.close();
  c.GetBlackboard().Set("Config.loaded", true);
  return Status::Success;
}