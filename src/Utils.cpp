#include "Utils.hpp"
#include "CustomTask.hpp"
#include "Regex.hpp"
#include "BotCommands.hpp"
#include "Discord.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
// #include "botcraft/AI/Tasks/AllTasks.hpp"
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

Position ParsePositionString(string posStr) {
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

// void CmdHandler(string text, Artist *artist) {
//   smatch matches;

//   if (regex_search(text, matches, HungryPattern)) {
//     cmdHungry(artist);
//   } else if (regex_search(text, matches, StopPattern)) {
//     cmdStop(matches, artist);
//   } else if (regex_search(text, matches, StartPattern)) {
//     cmdStart(matches, artist);
//   } else if (regex_search(text, matches, BarPattern)) {
//     cmdBar(artist);
//   } else if (regex_search(text, matches, CsafePattern)) {
//     artist->SendChatCommand(text);
//   } else if (regex_search(text, matches, CmdPattern)) {
//     cmdInGameCommand(matches, artist);
//   } else if (regex_search(text, matches, NamePattern)) {
//     string name = artist->GetNetworkManager()->GetMyName();

//     artist->SendChatMessage(name);
//   } else if (regex_search(text, matches, AssignmentPattern)) {
//     cmdAssignment(matches, artist);
//   } else if (regex_search(text, matches, WorkerPattern)) {
//     cmdWorker(matches, artist);
//   } else if (regex_search(text, matches, DutyPattern)) {
//     int workCol = artist->board.Get<int>("workCol", 0);
//     int workerNum = artist->board.Get<int>("workerNum", 1);
//     string info = "Max worker: " + to_string(workerNum) + ", work col: " + to_string(workCol);

//     artist->SendChatMessage(info);
//   } else if (regex_search(text, matches, DefaultSettingPattern)) {
//     cmdDefaultSetting(artist);
//   } else if (regex_search(text, matches, IngotPattern)) {
//     string rate = artist->board.Get<string>("ExchangeRate", "NOT_FOUND");
//     string info = "1 Villager Ingot = " + rate + " emerald.";

//     artist->SendChatMessage(info);
//   } else if (regex_search(text, matches, ChannelPattern)) {
//     string info = "Current channel: " + artist->board.Get<string>("ChannelNumber", "NOT_FOUND");

//     artist->SendChatMessage(info);
//   } else if (regex_search(text, matches, MovePattern)) {
//     cmdMove(matches, artist);
//   } else if (regex_search(text, matches, WaitingRoomPattern)) {
//     cmdWaitingRoom(matches, artist);
//   } else if (regex_search(text, matches, TpSuccessPattern)) {
//     cmdTpSuccess(artist);
//   } else if (regex_search(text, matches, TpHomePattern)) {
//     cmdTpHome(matches, artist);
//   }
// }

void MessageOutput(string text, Artist* artist) {
  if (artist->board.Get<bool>("use.dpp")) {
    DiscordBot& b = DiscordBot::getDiscordBot();
    b.sendDCMessage(text);
  } else {
    // Say(*artist, text);
  }
}