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

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;

void cmdHungry(Artist *artist);
void cmdStop(smatch matches, Artist *artist);
void cmdStart(smatch matches, Artist *artist);
void cmdBar(Artist *artist);
void cmdAssignment(smatch matches, Artist *artist);
void cmdWorker(smatch matches, Artist *artist);
void cmdDefaultSetting(Artist *artist);
void cmdMove(smatch matches, Artist *artist);
void cmdWaitingRoom(smatch matches, Artist *artist);
void cmdTpSuccess(Artist *artist);
void cmdTpHome(smatch matches, Artist *artist);

void cmdHandler(string text, Artist *artist) {
  smatch matches;
  Blackboard& bb = artist->GetBlackboard();

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
    string ingameCmd = matches[2];

    artist->SendChatCommand(ingameCmd);
  } else if (regex_search(text, matches, NamePattern)) {
    string name = artist->GetNetworkManager()->GetMyName();

    artist->SendChatMessage(name);
  } else if (regex_search(text, matches, AssignmentPattern)) {
    cmdAssignment(matches, artist);
  } else if (regex_search(text, matches, WorkerPattern)) {
    cmdWorker(matches, artist);
  } else if (regex_search(text, matches, DutyPattern)) {
    int workCol = (artist->hasWork) ? bb.Get<int>("workCol", 0) : any_cast<int>(artist->backup["workCol"]);
    int workerNum = (artist->hasWork) ? bb.Get<int>("workerNum", 1) : any_cast<int>(artist->backup["workerNum"]);
    string info = "Max worker: " + to_string(workerNum) + ", work col: " + to_string(workCol);

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, DefaultSettingPattern)) {
    cmdDefaultSetting(artist);
  } else if (regex_search(text, matches, IngotPattern)) {
    string rate = bb.Get<string>("ExchangeRate", "NOT_FOUND");
    string info = "1 Villager Ingot = " + rate + " emerald.";

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, ChannelPattern)) {
    string info = "Current channel: " + bb.Get<string>("ChannelNumber", "NOT_FOUND");

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, MovePattern)) {
    cmdMove(matches, artist);
  } else if (regex_search(text, matches, WaitingRoomPattern)) {
    cmdWaitingRoom(matches, artist);
  } else if (regex_search(text, matches, TpSuccessPattern)) {
    cmdTpSuccess(artist);
  } else if (regex_search(text, matches, TpHomePattern)) {
    cmdTpHome(matches, artist);
  }
}

void msgProcessor(string text, Artist *artist) {
  smatch match;
  if (regex_search(text, match, DiscordPattern)) {
    cout << "==Discord==" << text << endl;
    cmdHandler(text, artist);
  } else if (regex_search(text, match, SystemInfoPattern)) {
    cout << "==System Info==" << text << endl;
    cmdHandler(text, artist);
  } else if (regex_search(text, match, WaitingRoomPattern)) {
    cout << "==Wait Room==" << text << endl;
    cmdHandler(text, artist);
  } else if (regex_search(text, match, TpHomePattern)) {
    cout << "==TP Home==" << text << endl;
    cmdHandler(text, artist);
  } else {
    // cout << "==Other==" << text << endl;
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

std::future<void> Artist::waitTP(){
  return tpNotifier.add();
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

  // cout << "================" << endl;
  // cout << header << endl;
  // cout << "================" << endl;
  if (regex_search(header, match, FalloutTabPatternVer2)) {
    Blackboard& bb = this->GetBlackboard();
    bb.Set("ExchangeRate", match[14].str());
    bb.Set("ChannelNumber", match[15].str());
    bb.Set("CurrentPos", match[18].str());
  }
}

void Artist::Handle(ClientboundPlayerPositionPacket &msg) {
  ConnectionClient::Handle(msg);
  cout << GetTime() << "TP to position: " << msg.GetX() << ", " << msg.GetY() << ", " << msg.GetZ() << endl;
  cout << GetTime() << "Notify all listeners" << endl;
  while(tpNotifier.size() > 0){
    auto t = tpNotifier.pop();
    if(!t) break;
    t->set_value();
  }
  cout << GetTime() << "Finish notifying all listeners" << endl;
  cout << "=========================" << endl;
}

void cmdHungry(Artist *artist) {
  Status s = IsHungry(*artist, 15);

  cout << GetTime() << "Current food: " << artist->GetEntityManager()->GetLocalPlayer()->GetFood() << endl;
  if (s == Status::Success) {
    artist->SendChatMessage("I'm hungry.");
  } else {
    artist->SendChatMessage("I'm not hungry.");
  }
}

void cmdStop(smatch matches, Artist *artist) {
  string name = artist->GetNetworkManager()->GetMyName();
  if (string(matches[2]) != "all" && string(matches[2]) != name) return;
  
  artist->SendChatMessage("=== BOT STOP ===");
  // backup first, then set hasWork to false
  artist->Backup();
  artist->hasWork = false;
  artist->SetBehaviourTree(nullptr);
}

void cmdStart(smatch matches, Artist *artist) {
  string name = artist->GetNetworkManager()->GetMyName();
  if (string(matches[2]) != "all" && string(matches[2]) != name) return;

  cout << GetTime() << "=== BOT START ===" << endl;
  artist->SendChatMessage("=== BOT START ===");
  artist->hasWork = true;
  map<string, any> &initVal = artist->Recover();
  artist->SetBehaviourTree(FullTree(), initVal);
}

void cmdBar(Artist *artist) {
  vector<bool> xCheck = artist->GetBlackboard().Get<vector<bool>>("SliceDFS.xCheck", vector(128, false));
  int workers = artist->GetBlackboard().Get<int>("workerNum", 1);
  int col = artist->GetBlackboard().Get<int>("workCol", 0);
  int finish = 0;
  int duty = 0;

  for (int i = 0; i < xCheck.size(); i++) {
    if (i%workers != col) continue;
    duty++;
    if (xCheck[i]) finish++;
  }
  int ratio = finish * 20 / duty;
  double percent = static_cast<double>(finish) * 100 / duty;

  ostringstream bar;
  bar << "[" << string(ratio, '#') << string(20 - ratio, '-') << "]  "
      << fixed << setprecision(1) << percent << "%";
  artist->SendChatMessage(bar.str());
}

void cmdAssignment(smatch matches, Artist *artist) {
  Blackboard& bb = artist->GetBlackboard();
  string name = artist->GetNetworkManager()->GetMyName();
  string exp_user = matches[2];
  int col = stoi(matches[3]);

  if (exp_user != name) return;
  string info = "Assign user " + exp_user + " for column " + string(matches[3]);

  cout << GetTime() << info << endl;
  artist->SendChatMessage(info);
  if (artist->hasWork) bb.Set("workCol", col);
  else artist->backup["workCol"] = col;
}

void cmdWorker(smatch matches, Artist *artist) {
  Blackboard& bb = artist->GetBlackboard();
  int worker_num = stoi(matches[2]);
  string info = "Setup total worker " + string(matches[2]);

  cout << GetTime() << info << endl;
  artist->SendChatMessage(info);
  if (artist->hasWork) bb.Set("workerNum", worker_num);
  else artist->backup["workerNum"] = worker_num;
}

void cmdDefaultSetting(Artist *artist) {
  Blackboard& bb = artist->GetBlackboard();
  string info = "Reset workCol & workerNum value";

  artist->SendChatMessage(info);
  if (artist->hasWork) bb.Set("workCol", 0);
  else artist->backup["workCol"] = 0;
}

void cmdMove(smatch matches, Artist *artist) {
  int x = stoi(matches[2]), y = stoi(matches[3]), z = stoi(matches[4]);

  if (matches[1] == "bmove") {
    artist->SetBehaviourTree(BMoveTree(Position(x, y, z)));
  } else {
    map<string, any> &initVal = artist->Recover();
    artist->SetBehaviourTree(MoveTree(Position(x, y, z)), initVal);
  }
}

void cmdWaitingRoom(smatch matches, Artist *artist) {
  if (matches[1] == "wait") {
    artist->Backup();
    artist->inWaitingRoom = true;
    artist->SetBehaviourTree(nullptr);
    cout << GetTime() << "Transfered to waiting room, stop working..." << endl;
    // artist->SendChatMessage("Transfered to waiting room, stop working...");
  } else {
    if (artist->inWaitingRoom) artist->waitTpFinish = true;

    // string info = "Transfered to channel " + string(matches[1]);

    // artist->SendChatMessage(info);
  }
}

void cmdTpSuccess(Artist *artist) {
  if (artist->waitTpFinish) {
    artist->inWaitingRoom = false;
    artist->waitTpFinish = false;
    cout << GetTime() << "Back to work..." << endl;
    // artist->SendChatMessage("Back to work...");
    if (artist->hasWork) {
      artist->hasWork = true;
      map<string, any> &initVal = artist->Recover();
      artist->SetBehaviourTree(FullTree(), initVal);
    }
  }
}

void cmdTpHome(smatch matches, Artist *artist) {
  Blackboard& bb = artist->GetBlackboard();
  string homeName = bb.Get<string>("home", "mapart");

  if (string(matches[1]) == homeName) {
    bb.Set("GetHome", true);
    cout << GetTime() << "Bot teleport to home." << endl;
  }
}