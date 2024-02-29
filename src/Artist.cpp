#include <iostream>
#include <fstream>
#include "Artist.hpp"
#include "BotCommands.hpp"
#include "Regex.hpp"
#include "Utils.hpp"

using namespace Botcraft;
using namespace ProtocolCraft;

void msgProcessor(std::string text, Artist *artist) {
  std::smatch match;
  if (regex_search(text, match, DiscordPattern)) {
    std::cout << "==Discord==" << GetTime() << text << std::endl;
    CmdHandler(text, artist);
  } else if (regex_search(text, match, SystemInfoPattern)) {
    std::cout << "==System Info==" << GetTime() << text << std::endl;
    CmdHandler(text, artist);
  } else if (regex_search(text, match, WaitingRoomPattern)) {
    std::cout << "==Wait Room==" << GetTime() << text << std::endl;
    CmdHandler(text, artist);
  } else if (regex_search(text, match, TpHomePattern)) {
    std::cout << "==TP Home==" << GetTime() << text << std::endl;
    CmdHandler(text, artist);
  } else {
    // std::cout << "==Other==" << text << std::endl;
  }
}

Artist::Artist(const bool use_renderer, std::string path) : SimpleBehaviourClient(use_renderer), finder(this) {
  configPath = path;
  inWaitingRoom = false;
  waitTpFinish = false;
  hasWork = false;
  tpID = 0;

  std::ifstream file(configPath, std::ios::in);

  if (!file.is_open()) {
    std::cerr << GetTime() << "Unable to open file: " + configPath << std::endl;
  }

  std::string line;
  while (getline(file, line)) {
    // if line start with '#' or is empty, skip
    if (line.empty() || line[0] == '#') continue;

    std::istringstream iss(line);
    std::string key, value;
    getline(iss, key, '=') && getline(iss, value);
    if (key == "anchor") {
      Position anchor = ParsePositionString(value);
      board.Set("anchor", anchor);
    } else if (key == "dctoken" && value != "") {
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
      std::cout << "TP Home command: " << value << std::endl;
      board.Set("home", value);
    } else if (key == "retry") {
      board.Set("retry", stoi(value));
    } else if (key == "neighbor") {
      board.Set("neighbor", value == "true");
    } else {
      std::vector<Position> posVec;
      std::istringstream _iss(value);
      std::string posGroup;
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
  std::string text = msg.GetBody().GetContent();

  CmdHandler(text, this);
}

void Artist::waitTP(){
  tpNotifier.wait();
}

std::size_t Artist::getTPID(){
  return tpID;
}

void Artist::Handle(ClientboundSystemChatPacket &msg) {
  ManagersClient::Handle(msg);
  std::string text = msg.GetContent().GetText();

  msgProcessor(text, this);
}

void Artist::Handle(ClientboundTabListPacket &msg) {
  ManagersClient::Handle(msg);

  std::string header = msg.GetHeader().GetText();
  std::string footer = msg.GetFooter().GetText();
  std::smatch match;
  header.erase(remove(header.begin(), header.end(), ','), header.end());

  board.Set("Header", header);
  if (regex_search(header, match, FalloutTabPatternVer2)) {
    board.Set("ExchangeRate", match[14].str());
    board.Set("ChannelNumber", match[15].str());
    board.Set("CurrentPos", match[18].str());
  }
}

void Artist::Handle(ClientboundPlayerPositionPacket &msg) {
  ConnectionClient::Handle(msg);
  tpID++;
  std::cout << GetTime() << "TP to position: " << msg.GetX() << ", " << msg.GetY() << ", " << msg.GetZ() << std::endl;
  std::cout << GetTime() << "Notify all listeners" << std::endl;
  tpNotifier.notify_all();
  std::cout << GetTime() << "Finish notifying all listeners" << std::endl;
  std::cout << "=========================" << std::endl;
}