#include "Utils.hpp"
#include "CustomTask.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include <iomanip>
#include <string>
#include <sstream>

using namespace std;
using namespace Botcraft;
using namespace ProtocolCraft;

string GetTime() {
  auto t = time(nullptr);
  auto tm = *localtime(&t);
  ostringstream oss;
  oss << "[" << put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
  return oss.str();
}

string GetWorldBlock(BehaviourClient& c, Position pos) {
  shared_ptr<World> world = c.GetWorld();

  string curBlockName = "minecraft:air";
  const Blockstate* block = world->GetBlock(pos);

  if (!block) {
    // it is a unload block
    if (!world->IsLoaded(pos)) {
      FindPathAndMove(c, pos,  5, 5, -1, -1, 5, 5,  -1, -1, -1, -1, -1, -1);

      block = world->GetBlock(pos);
      if (block) curBlockName = block->GetName();
    }
  } else {
    curBlockName = block->GetName();
  }

  return curBlockName;
}

string GetTaskType(const string &worldBlockName, const string &nbtBlockName) {
  string taskType = "None";
  if (nbtBlockName != "minecraft:air" && worldBlockName == "minecraft:air") {
    taskType = "Place";
  } else if (worldBlockName != "minecraft:air" && nbtBlockName != worldBlockName) {
    taskType = "Dig";
  }

  return taskType;
}

int GetItemAmount(BehaviourClient& c, string itemName) {
  shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  shared_ptr<Window> playerInv = inventory_manager->GetPlayerInventory();

  int amount = 0;

  for (short i = Window::INVENTORY_STORAGE_START; i <= Window::INVENTORY_HOTBAR_START+8; i++) {
    const Slot& slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) continue;

    string name = AssetsManager::getInstance().Items().at(slot.GetItemID())->GetName();
    if (name == itemName) amount += AssetsManager::getInstance().Items().at(slot.GetItemID())->GetStackSize();
  }

  return amount;
}