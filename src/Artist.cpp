#include "Artist.hpp"
#include "CustomSubTree.hpp"
#include "CustomTask.hpp"
#include "Regex.hpp"
#include "Utils.hpp"
#include "botcraft/AI/BehaviourTree.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Network/NetworkManager.hpp"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;
using json = nlohmann::json;

void cmdHandler(string text, Artist *artist) {
  smatch matches;
  if (regex_search(text, matches, HungryPattern)) {
    Status s = IsHungry(*artist, 15);

    cout << GetTime() << "Current food: " << artist->GetEntityManager()->GetLocalPlayer()->GetFood() << endl;
    if (s == Status::Success) {
      artist->SendChatMessage("I'm hungry.");
    } else {
      artist->SendChatMessage("I'm not hungry.");
    }
  } else if (regex_search(text, matches, StopPattern)) {
    string name = artist->GetNetworkManager()->GetMyName();
    if (string(matches[2]) != "all" && string(matches[2]) != name) return;
    
    artist->SendChatMessage("=== BOT STOP ===");
    // backup first, then set hasWork to false
    artist->Backup();
    artist->hasWork = false;
    artist->SetBehaviourTree(nullptr);
  } else if (regex_search(text, matches, StartPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string name = artist->GetNetworkManager()->GetMyName();
    if (string(matches[2]) != "all" && string(matches[2]) != name) return;

    cout << GetTime() << "=== BOT START ===" << endl;
    artist->SendChatMessage("=== BOT START ===");
    artist->hasWork = true;
    map<string, any> &initVal = artist->Recover();
    artist->SetBehaviourTree(FullTree(), initVal);
  } else if (regex_search(text, matches, BarPattern)) {
    vector<bool> xCheck = artist->GetBlackboard().Get<vector<bool>>("SliceDFS.xCheck", vector(128, false));
    int workers = artist->GetBlackboard().Get<int>("workerNum", 1);
    int col = artist->GetBlackboard().Get<int>("workCol", 0);
    int finish = 0;

    for (int i = 0; i < xCheck.size(); i++) {
      if (i%workers != col) continue;
      if (xCheck[i]) finish++;
    }
    int ratio = finish * 20 / 128;
    double percent = static_cast<double>(finish) * 100 / 128;

    ostringstream bar;
    bar << "[" << string(ratio, '#') << string(20 - ratio, '-') << "]  "
        << fixed << setprecision(1) << percent << "%";
    artist->SendChatMessage(bar.str());
  } else if (regex_search(text, matches, CsafePattern)) {
    artist->SendChatCommand(text);
  } else if (regex_search(text, matches, CmdPattern)) {
    string ingameCmd = matches[2];

    artist->SendChatCommand(ingameCmd);
  } else if (regex_search(text, matches, NamePattern)) {
    string name = artist->GetNetworkManager()->GetMyName();

    artist->SendChatMessage(name);
  } else if (regex_search(text, matches, AssignmentPattern)) {
    string name = artist->GetNetworkManager()->GetMyName();
    string exp_user = matches[2];
    int col = stoi(matches[3]);

    if (exp_user != name) return;
    Blackboard& bb = artist->GetBlackboard();
    string info = "Assign user " + exp_user + " for column " + string(matches[3]);

    cout << GetTime() << info << endl;
    artist->SendChatMessage(info);
    if (artist->hasWork) bb.Set("workCol", col);
    else artist->backup["workCol"] = col;
  } else if (regex_search(text, matches, WorkerPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    int worker_num = stoi(matches[2]);
    string info = "Setup total worker " + string(matches[2]);

    cout << GetTime() << info << endl;
    artist->SendChatMessage(info);
    if (artist->hasWork) bb.Set("workerNum", worker_num);
    else artist->backup["workerNum"] = worker_num;
  } else if (regex_search(text, matches, DutyPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    int workCol = (artist->hasWork) ? bb.Get<int>("workCol", 0) : any_cast<int>(artist->backup["workCol"]);
    int workerNum = (artist->hasWork) ? bb.Get<int>("workerNum", 1) : any_cast<int>(artist->backup["workerNum"]);
    string info = "Max worker: " + to_string(workerNum) + ", work col: " + to_string(workCol);

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, DefaultSettingPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string info = "Reset workCol & workerNum value";

    artist->SendChatMessage(info);
    if (artist->hasWork) bb.Set("workCol", 0);
    else artist->backup["workCol"] = 0;
  } else if (regex_search(text, matches, IngotPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string rate = bb.Get<string>("ExchangeRate", "NOT_FOUND");
    string info = "1 Villager Ingot = " + rate + " emerald.";

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, ChannelPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string info = "Current channel: " + bb.Get<string>("ChannelNumber", "NOT_FOUND");

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, MovePattern)) {
    int x = stoi(matches[2]), y = stoi(matches[3]), z = stoi(matches[4]);

    if (matches[1] == "bmove") {
      artist->SetBehaviourTree(BMoveTree(Position(x, y, z)));
    } else {
      map<string, any> &initVal = artist->Recover();
      artist->SetBehaviourTree(MoveTree(Position(x, y, z)), initVal);
    }
  } else if (regex_search(text, matches, WaitingRoomPattern)) {
    if (matches[1] == "wait") {
      artist->Backup();
      artist->inWaitingRoom = true;
      artist->SetBehaviourTree(nullptr);
      artist->SendChatMessage("Transfered to waiting room, stop working...");
    } else {
      if (artist->inWaitingRoom) artist->waitTpFinish = true;

      string info = "Transfered to channel " + string(matches[1]);

      artist->SendChatMessage(info);
    }
  } else if (regex_search(text, matches, TpSuccessPattern)) {
    if (artist->waitTpFinish) {
      artist->inWaitingRoom = false;
      artist->waitTpFinish = false;
      artist->SendChatMessage("Back to work...");
      if (artist->hasWork) {
        artist->hasWork = true;
        map<string, any> &initVal = artist->Recover();
        artist->SetBehaviourTree(FullTree(), initVal);
      }
    }
  }
}

void msgProcessor(string text, Artist *artist) {
  cout << text << endl;
  smatch match;
  if (regex_search(text, match, DiscordPattern)) {
    cout << text << endl;
    cmdHandler(text, artist);
  } else if (regex_search(text, match, SystemInfoPattern)) {
    cout << text << endl;
    cmdHandler(text, artist);
  } else if (regex_search(text, match, WaitingRoomPattern)) {
    cout << text << endl;
    cmdHandler(text, artist);
  }
}

Artist::Artist(const bool use_renderer, string path) : SimpleBehaviourClient(use_renderer), finder(shared_ptr<BehaviourClient>(this)) {
  configPath = path;
  inWaitingRoom = false;
  waitTpFinish = false;
  hasWork = false;
  vector<string> key{"configPath", "pathFinder", "workerNum", "workCol"};
  vector<any> initVal{path, finder, 1, 0};
  for (int i = 0; i < key.size(); i++) {
    backup[key[i]] = initVal[i];
  }
}

Artist::~Artist() {}

// Local server chat
void Artist::Handle(ClientboundPlayerChatPacket &msg) {
  ManagersClient::Handle(msg);
  string text = msg.GetBody().GetContent();

  cout << text << endl;
  cmdHandler(text, this);
}

void Artist::Backup() {
  if (!hasWork) return;
  Blackboard& bb = GetBlackboard();
  vector<string> keys{"workerNum", "workCol", "configPath", "pathFinder"};
  vector<string> types{"int", "int", "string", "PathFinder"};

  for (int i = 0; i < keys.size(); i++) {
    if (types[i] == "int") {
      backup[keys[i]] = bb.Get<int>(keys[i]);
    } else if (types[i] == "string") {
      backup[keys[i]] = bb.Get<string>(keys[i]);
    } else if (types[i] == "PathFinder") {
      backup[keys[i]] = bb.Get<PathFinder>(keys[i]);
    }
  }
}

map<string, any>& Artist::Recover() {
  return backup;
}

void Artist::Handle(ClientboundSystemChatPacket &msg) {
    ManagersClient::Handle(msg);
    string text = msg.GetContent().GetText();

    msgProcessor(text, this);
}

void Artist::Handle(ClientboundTabListPacket &msg) {
    ManagersClient::Handle(msg);

    string header = msg.GetHeader().GetText();
    string footer = msg.GetFooter().GetText();
    smatch match;
    header.erase(remove(header.begin(), header.end(), ','), header.end());

    if (regex_search(header, match, FalloutTabPattern)) {
      Blackboard& bb = this->GetBlackboard();
      bb.Set("ExchangeRate", match[12].str());
      bb.Set("ChannelNumber", match[13].str());
      bb.Set("CurrentPos", match[15].str());
    }
}