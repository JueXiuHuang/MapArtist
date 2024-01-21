#include "Artist.hpp"
#include "BotCommands.hpp"
#include "Regex.hpp"
#include "Utils.hpp"
#include <iostream>

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;

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
    cmdInGameCommand(matches, artist);
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

Artist::Artist(const bool use_renderer, string path) : SimpleBehaviourClient(use_renderer), finder(this) {
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

void Artist::waitTP(){
  tpNotifier.wait();
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
  tpNotifier.notify_all();
  cout << GetTime() << "Finish notifying all listeners" << endl;
  cout << "=========================" << endl;
}