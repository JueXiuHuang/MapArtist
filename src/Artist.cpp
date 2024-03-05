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

Artist::Artist(const bool use_renderer, Config _conf)
    : SimpleBehaviourClient(use_renderer),
      finder(static_cast<Botcraft::BehaviourClient *>(this)) {
  conf = _conf;
  inWaitingRoom = false;
  waitTpFinish = false;
  hasWork = false;
  needRestart = false;
  tpID = 0;
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
