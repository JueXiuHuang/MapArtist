#include <iomanip>
#include <string>
#include <sstream>
#include <botcraft/Game/AssetsManager.hpp>
#include <botcraft/Game/Inventory/InventoryManager.hpp>
#include <botcraft/Game/Inventory/Window.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include "Utils.hpp"
#include "CustomTask.hpp"
#include "Regex.hpp"
#include "BotCommands.hpp"
#include "Discord.hpp"
#include "Constants.hpp"

using namespace Botcraft;
using namespace ProtocolCraft;

std::string GetTime() {
  auto t = time(nullptr);
  auto tm = *localtime(&t);
  std::ostringstream oss;
  oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
  return oss.str();
}

std::string GetWorldBlock(BehaviourClient& c, Position absolutePos) {
  std::shared_ptr<World> world = c.GetWorld();

  std::string curBlockName = "minecraft:air";
  const Blockstate* block = world->GetBlock(absolutePos);

  if (!block) {
    // it is a unload block
    if (!world->IsLoaded(absolutePos)) {
      FindPathAndMove(c, absolutePos,  5, 5, -1, -1, 5, 5,  -1, -1, -1, -1, -1, -1);

      block = world->GetBlock(absolutePos);
      if (block) curBlockName = block->GetName();
    }
  } else {
    curBlockName = block->GetName();
  }

  return curBlockName;
}

std::string GetTaskType(const std::string &worldBlockName, const std::string &nbtBlockName) {
  std::string taskType = "None";
  if (nbtBlockName != "minecraft:air" && worldBlockName == "minecraft:air") {
    taskType = "Place";
  } else if (worldBlockName != "minecraft:air" && nbtBlockName != worldBlockName) {
    taskType = "Dig";
  }

  return taskType;
}

int GetItemAmount(BehaviourClient& c, std::string itemName) {
  std::shared_ptr<InventoryManager> inventory_manager = c.GetInventoryManager();
  std::shared_ptr<Window> playerInv = inventory_manager->GetPlayerInventory();

  int amount = 0;

  for (short i = Window::INVENTORY_STORAGE_START; i <= Window::INVENTORY_HOTBAR_START+8; i++) {
    const Slot& slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) continue;

    std::string name = AssetsManager::getInstance().Items().at(slot.GetItemID())->GetName();
    if (name == itemName) amount += AssetsManager::getInstance().Items().at(slot.GetItemID())->GetStackSize();
  }

  return amount;
}

Position ParsePositionString(std::string posStr) {
  std::vector<int> integers;
  std::istringstream iss(posStr);
  std::string token;

  while (std::getline(iss, token, ',')) {
    try {
      int num = std::stoi(token);
      integers.push_back(num);
    } catch (const std::exception& e) {
      std::cerr << GetTime() << "Invalid position: " << token << std::endl;
    }
  }
  Position pos(integers);

  return pos;
}

void CmdHandler(std::string text, Artist *artist) {
  std::smatch matches;

  if (regex_search(text, matches, HungryPattern)) {
    cmdHungry(artist);
  } else if (regex_search(text, matches, StopPattern)) {
    cmdStop(matches, artist);
  } else if (regex_search(text, matches, StartPattern)) {
    cmdStart(matches, artist);
  } else if (regex_search(text, matches, BarPattern)) {
    cmdBar(artist);
  } else if (regex_search(text, matches, CsafePattern)) {
    artist->SendChatCommand(text);
  } else if (regex_search(text, matches, CmdPattern)) {
    cmdInGameCommand(matches, artist);
  } else if (regex_search(text, matches, NamePattern)) {
    std::string name = artist->GetNetworkManager()->GetMyName();

    MessageOutput(name, artist);
  } else if (regex_search(text, matches, AssignmentPattern)) {
    cmdAssignment(matches, artist);
  } else if (regex_search(text, matches, WorkerPattern)) {
    cmdWorker(matches, artist);
  } else if (regex_search(text, matches, DutyPattern)) {
    int workCol = artist->board.Get<int>(KeyWorkerCol, 0);
    int workerNum = artist->board.Get<int>(KeyWorkerCount, 1);
    std::string info = "Max worker: " + std::to_string(workerNum) + ", work col: " + std::to_string(workCol);

    MessageOutput(info, artist);
  } else if (regex_search(text, matches, DefaultSettingPattern)) {
    cmdDefaultSetting(artist);
  } else if (regex_search(text, matches, IngotPattern)) {
    std::string rate = artist->board.Get<std::string>(KeyIngotRate, "NOT_FOUND");
    std::string info = "1 Villager Ingot = " + rate + " emerald.";

    MessageOutput(info, artist);
  } else if (regex_search(text, matches, ChannelPattern)) {
    std::string info = "Current channel: " + artist->board.Get<std::string>(KeyCurrChNum, "NOT_FOUND");

    MessageOutput(info, artist);
  } else if (regex_search(text, matches, MovePattern)) {
    cmdMove(matches, artist);
  } else if (regex_search(text, matches, WaitingRoomPattern)) {
    cmdWaitingRoom(matches, artist);
  } else if (regex_search(text, matches, TpSuccessPattern)) {
    cmdTpSuccess(artist);
  } else if (regex_search(text, matches, TpHomePattern)) {
    cmdTpHome(matches, artist);
  } else if (regex_search(text, matches, DetailPattern)) {
    cmdDetail(matches, artist);
  }
}

void MessageOutput(std::string text, Artist* artist) {
  if (artist->board.Get<bool>(KeyUseDc, false)) {
    DiscordBot& b = DiscordBot::getDiscordBot();
    b.sendDCMessage(text);
  } else {
    artist->SendChatMessage(text);
  }
}

void ListPlayerInventory(Artist* artist) {
  std::shared_ptr<InventoryManager> inventory_manager = artist->GetInventoryManager();
  std::shared_ptr<Window> playerInv = inventory_manager->GetPlayerInventory();
  
  std::cout << GetTime() << "========== LIST START ==========" << std::endl;
  for (short i = Window::INVENTORY_STORAGE_START; i < Window::INVENTORY_OFFHAND_INDEX+1; i++) {
    const Slot& slot = playerInv->GetSlot(i);
    if (slot.IsEmptySlot()) {
      std::cout << GetTime() << "Slot " << i << ": " << std::endl;
    } else {
      std::cout << GetTime() << "Slot " << i << ": " 
        << AssetsManager::getInstance().Items().at(slot.GetItemID())->GetName()
        << " x " << static_cast<int>(slot.GetItemCount()) << std::endl;
    }
  }
  std::cout << GetTime() << "======= LIST END =======" << std::endl;
} 