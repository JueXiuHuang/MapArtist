#include "CustomTask.hpp"
#include "Algorithm.hpp"
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
    GoTo(c, chests[i], 1, 1, 1, 10);
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
    GoTo(c, chest, 1, 1, 1, 10);
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

Status TaskPrioritize(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  string algo = blackboard.Get<string>("prioritize");
  if (algo == "bfs") {
    SimpleBFS(c);
  } else if (algo == "dfs") {
    SimpleDFS(c);
  } else if (algo == "slice_dfs") {
    SliceDFS(c);
  } else {
    LOG_ERROR("Get unrecognized prioritize method: " << algo);
  }

  return Status::Success;
}

Status CollectAllMaterial(BehaviourClient& c) {
  LOG_INFO("Trying to collect material...");
  Blackboard& blackboard = c.GetBlackboard();
  map<string, int, MaterialCompare> itemCounter = blackboard.Get<map<string, int, MaterialCompare>>("itemCounter");

  for (auto item : itemCounter) {
    CollectSingleMaterial(c, item.first, item.second);
  }
  return Status::Success;
}

Status CollectSingleMaterial(BehaviourClient& c, string itemName, int needed) {
  LOG_INFO(endl << "Collecting " << itemName << " for " << needed);
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  vector<Position> availableChests = blackboard.Get<vector<Position>>("chest:"+itemName);

  bool get_all_material = false;
  for (auto chest : availableChests) {
    LOG_INFO("========== CHEST ==========");
    SortInventory(c);
    GoTo(c, chest, 1, 1, 1, 10);
    if (OpenContainer(c, chest) == Status::Failure) continue;
    
    int _need = needed;
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
    }

    while (!canTake.empty() && !canPut.empty() && !get_all_material) {
      LOG_INFO("Swap from ID " << canTake.front() << " to ID " << canPut.front());
      Status swapResult = SwapItemsInContainer(c, containerId, canTake.front(), canPut.front());
      if (swapResult == Status::Failure){  // if failed, wait for a while and retry
        Utilities::SleepFor(chrono::milliseconds(500));
        LOG_INFO("Take " << itemName << " Failed");
        continue;
      }
      inventory_manager->GetMutex().lock();
      int took_amount = container->GetSlot(canPut.front()).GetItemCount();
      inventory_manager->GetMutex().unlock();
      LOG_INFO("Take " << itemName << " for " << took_amount);
      _need -= took_amount;
      canPut.pop();
      canTake.pop();

      if (_need < 1) get_all_material = true;
    }

    {
      LOG_INFO("========== LIST ==========");
      lock_guard<mutex> lock(inventory_manager->GetMutex());
      const short playerInvStart = container->GetFirstPlayerInventorySlot();
      for (auto p : container->GetSlots()){
        if (p.first >= playerInvStart && !p.second.IsEmptySlot()){
          LOG_INFO("Slot " << p.first << ": " << AssetsManager::getInstance().Items().at(p.second.GetItemID())->GetName()
            << " x " << static_cast<int>(p.second.GetItemCount()));
        }
      }
      LOG_INFO("======= LIST CLOSE =======");
    }

    CloseContainer(c, containerId);
    LOG_INFO("======= CHEST CLOSE =======");
    if (get_all_material) break;
  }

  if (!get_all_material) {
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
  int retry_times = blackboard.Get<int>("retry");
  vector<Position> offsets {Position(1, 0, 0), Position(-1, 0, 0), Position(0, 0, 1), Position(0, 0, -1)};
  while (!qTaskPosition.empty() && !qTaskType.empty() && !qTaskName.empty()) {
    Position taskPos = qTaskPosition.front();
    string taskType = qTaskType.front();
    string blockName = qTaskName.front();
    for (int i = 0; i < retry_times; i++) {
      Status exec_result = ExecuteTask(c, taskType, taskPos, blockName);
      if (exec_result == Status::Success) break;
      else {
        LOG_WARNING("Task fail, move to another position and try again...");
        GoTo(c, taskPos+offsets[i%offsets.size()]);
      }
    }
    
    qTaskPosition.pop();
    qTaskType.pop();
    qTaskName.pop();
  }

  return Status::Success;
}

Status ExecuteTask(BehaviourClient& c, string action, Position blockPos, string blockName) {
  LOG_INFO( "Task:" << setw(5) << action <<
          ", Block Name:" << setw(32) << blockName <<
          ", Position:" << blockPos);
  // Stop sprinting when exiting this function (in case we don't sprint, it's a no-op)
  Utilities::OnEndScope stop_sprinting([&]() { StopSprinting(c); });
  // Start sprinting
  StartSprinting(c);
  Blackboard& board = c.GetBlackboard();

  GoTo(c, blockPos, 2, 2, 2, 10);
  if (action == "Dig") {
    return Dig(c, blockPos, true);
  } else if (action == "Place") {
    return PlaceBlock(c, blockName, blockPos, nullopt, true, true);
  }

  LOG_WARNING("Unknown task in ExecuteNextTask");
  return Status::Failure;
}

Status check(BehaviourClient& c) {
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

  for (int x = start.x; x < end.x; x++) {
    world_pos.x = x;
    target_pos.x = x - start.x;
    for (int y = start.y; y < end.y; y++) {
      world_pos.y = y;
      target_pos.y = y - start.y;
      for (int z = start.z; z < end.z; z++) {
        world_pos.z = z;
        target_pos.z = z - start.z;

        const short target_id = target[target_pos.x][target_pos.y][target_pos.z];
        string target_name = palette.at(target_id);
        
        string block_name = "minecraft:air";
        {
          lock_guard<mutex> lock(world->GetMutex());
          const Block* block = world->GetBlock(world_pos);

          if (!world->IsLoaded(world_pos)) {
            continue;
          } else if (block) {
            block_name = block->GetBlockstate()->GetName();
          }
        }

        if (block_name == "minecraft:air" && target_name == "minecraft:air") {
          // continue if it is a air block
          continue;
        } else if (block_name == "minecraft:air" && target_name != "minecraft:air") {
          // Found air in real world, but it should be something else
          return Status::Failure;
        } else if (block_name != "minecraft:air" && target_name == "minecraft:air") {
          // Found something else, but it should be air.
          return Status::Failure;
        } else if (block_name != target_name) {
          // The name of block not match.
          return Status::Failure;
        }
      }
    }
  }
  return Status::Success;
}

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

  vector<Position> checkpoints {Position(40, 10, 40), Position(80, 10, 40), Position(40, 10, 80), Position(80, 10, 80)};

  const bool log_details = false;
  const bool log_errors = true;
  const bool full_check = true;

  for (auto cp : checkpoints) {
    GoTo(c, anchor+cp, 4, 4, 4, 10);
    if (check(c) == Status::Failure) return Status::Failure;
  }

  return Status::Success;
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
    needed << setw(35) << palette[it->first] << "----" << it->second << "\n";
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
  Blackboard& blackboard = c.GetBlackboard();
  const std::string &configPath = blackboard.Get<std::string>("configPath");
  ifstream file(configPath, std::ios::in);

  if (!file.is_open()) {
    cerr << "Unable to open file: " + configPath << endl;
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
    } else if (key == "retry") {
      blackboard.Set("retry", stoi(value));
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