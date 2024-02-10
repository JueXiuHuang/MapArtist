#include "Artist.hpp"
#include "BotCommands.hpp"
#include "Regex.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;

void cmdHandler(string text, Artist *artist) {
  smatch matches;

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
    int workCol = artist->board.Get<int>("workCol", 0);
    int workerNum = artist->board.Get<int>("workerNum", 1);
    string info = "Max worker: " + to_string(workerNum) + ", work col: " + to_string(workCol);

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, DefaultSettingPattern)) {
    cmdDefaultSetting(artist);
  } else if (regex_search(text, matches, IngotPattern)) {
    string rate = artist->board.Get<string>("ExchangeRate", "NOT_FOUND");
    string info = "1 Villager Ingot = " + rate + " emerald.";

    artist->SendChatMessage(info);
  } else if (regex_search(text, matches, ChannelPattern)) {
    string info = "Current channel: " + artist->board.Get<string>("ChannelNumber", "NOT_FOUND");

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

  ifstream file(configPath, ios::in);

  if (!file.is_open()) {
    cerr << GetTime() << "Unable to open file: " + configPath << endl;
  }

  string line;
  while (getline(file, line)) {
    // if line start with '#' or is empty, skip
    if (line.empty() || line[0] == '#') continue;

    istringstream iss(line);
    string key, value;
    getline(iss, key, '=') && getline(iss, value);
    if (key == "anchor") {
      Position anchor = ParsePositionString(value);
      board.Set("anchor", anchor);
    } else if (key == "dctoken" && value != "") {
      cout << "hello" << endl;
      board.Set("dctoken", value);
      board.Set("use.dpp", true);
    } else if (key == "channelid") {
      board.Set("dcchannel", value);
    } else if (key == "nbt") {
      board.Set("nbt", value);
    } else if (key == "tempblock") {
      board.Set("tempblock", value);
    } else if (key == "prioritize") {
      board.Set("prioritize", value);
    } else if (key == "home") {
      cout << "TP Home command: " << value << endl;
      board.Set("home", value);
    } else if (key == "retry") {
      board.Set("retry", stoi(value));
    } else if (key == "neighbor") {
      board.Set("neighbor", value == "true");
    } else {
      vector<Position> posVec;
      istringstream _iss(value);
      string posGroup;
      while (getline(_iss, posGroup, ';')) {
        Position chestPos = ParsePositionString(posGroup);
        posVec.push_back(chestPos);
      }
      board.Set("chest:" + key, posVec);
    }
  }

  file.close();
  board.Set("Config.loaded", true);
}

Artist::~Artist() {}

// Local server chat
void Artist::Handle(ClientboundPlayerChatPacket &msg) {
  ManagersClient::Handle(msg);
  string text = msg.GetBody().GetContent();

  cmdHandler(text, this);
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