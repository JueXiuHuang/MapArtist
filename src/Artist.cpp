// Copyright 2024 JueXiuHuang, ldslds449

#include "./Artist.hpp"  // NOLINT

#include <fstream>
#include <iostream>

#include "./BotCommands.hpp"
#include "./Constants.hpp"
#include "./Regex.hpp"
#include "./Utils.hpp"

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

Artist::Artist(const bool use_renderer, std::string path)
    : SimpleBehaviourClient(use_renderer),
      finder(static_cast<Botcraft::BehaviourClient *>(this)) {
  configPath = path;
  inWaitingRoom = false;
  waitTpFinish = false;
  hasWork = false;
  needRestart = false;
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
      Botcraft::Position anchor = ParsePositionString(value);
      board.Set(KeyAnchor, anchor);
    } else if (key == "dctoken" && value != "") {
      board.Set(KeyDcToken, value);
      board.Set(KeyUseDc, true);
    } else if (key == "channelid") {
      board.Set(KeyDcChanID, value);
    } else if (key == "nbt") {
      board.Set(KeyNbt, value);
    } else if (key == "tempblock") {
      board.Set(KeyTmpBlock, value);
    } else if (key == "prioritize") {
      board.Set(KeyAlgorithm, value);
    } else if (key == "home") {
      std::cout << "TP Home command: " << value << std::endl;
      board.Set(KeyHomeCmd, value);
    } else if (key == "retry") {
      board.Set(KeyRetry, stoi(value));
    } else if (key == "neighbor") {
      board.Set(KeyNeighbor, value == "true");
    } else {
      std::vector<Botcraft::Position> posVec;
      std::istringstream _iss(value);
      std::string posGroup;
      while (getline(_iss, posGroup, ';')) {
        Botcraft::Position chestPos = ParsePositionString(posGroup);
        posVec.push_back(chestPos);
      }
      board.Set("chest:" + key, posVec);
    }
  }

  file.close();
  board.Set(KeyConfigLoaded, true);
}

Artist::~Artist() {}

// Local server chat
void Artist::Handle(ProtocolCraft::ClientboundPlayerChatPacket &msg) {
  ManagersClient::Handle(msg);
  std::string text = msg.GetBody().GetContent();

  CmdHandler(text, this);
}

void Artist::waitTP() { tpNotifier.wait(); }

std::size_t Artist::getTPID() { return tpID; }

bool Artist::getNeedRestart() { return needRestart; }

void Artist::setNeedRestart(const bool &restart) { needRestart = restart; }

void Artist::Handle(ProtocolCraft::ClientboundSystemChatPacket &msg) {
  ManagersClient::Handle(msg);
  std::string text = msg.GetContent().GetText();

  msgProcessor(text, this);
}

void Artist::Handle(ProtocolCraft::ClientboundTabListPacket &msg) {
  ManagersClient::Handle(msg);

  std::string header = msg.GetHeader().GetText();
  std::string footer = msg.GetFooter().GetText();
  std::smatch match;
  header.erase(remove(header.begin(), header.end(), ','), header.end());

  board.Set(KeyHeader, header);
  if (regex_search(header, match, FalloutTabPatternVer2)) {
    board.Set(KeyIngotRate, match[14].str());
    board.Set(KeyCurrChNum, match[15].str());
    board.Set(KeyCurrPos, match[18].str());
  }
}

void Artist::Handle(ProtocolCraft::ClientboundPlayerPositionPacket &msg) {
  ConnectionClient::Handle(msg);
  tpID++;
  std::cout << GetTime() << "TP to position: " << msg.GetX() << ", "
            << msg.GetY() << ", " << msg.GetZ() << std::endl;
  std::cout << GetTime() << "Notify all listeners" << std::endl;
  tpNotifier.notify_all();
  std::cout << GetTime() << "Finish notifying all listeners" << std::endl;
  std::cout << "=========================" << std::endl;
}

void Artist::Handle(ProtocolCraft::ClientboundDisconnectPacket &msg) {
  ConnectionClient::Handle(msg);
  needRestart = true;
}
