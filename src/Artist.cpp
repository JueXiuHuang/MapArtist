#include "Artist.hpp"
#include "CustomSubTree.hpp"
#include "botcraft/AI/BehaviourTree.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Network/NetworkManager.hpp"
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;
using json = nlohmann::json;

void cmdHandler(string cmd, Artist *artist) {
  if (cmd == "hungry") {
    Status s = IsHungry(*artist, 15);
    cout << "Current food: " << artist->GetEntityManager()->GetLocalPlayer()->GetFood() << endl;
    if (s == Status::Success) {
      artist->SendChatMessage("I'm hungry.");
    } else {
      artist->SendChatMessage("I'm not hungry.");
    }
  } else if (cmd == "stop") {
    artist->SendChatMessage("=== BOT STOP ===");
    artist->SetBehaviourTree(nullptr);
  } else if (cmd == "start") {
    Blackboard& bb = artist->GetBlackboard();
    int workerNum = bb.Get<int>("workerNum", 1);
    int workCol = bb.Get<int>("workCol", 0);
    artist->SendChatMessage("=== BOT START ===");
    artist->SetBehaviourTree(FullTree(), {{"configPath", artist->configPath},
                                          {"pathFinder", artist->finder}, 
                                          {"workerNum", workerNum},
                                          {"workCol", workCol}});
  } else if (cmd == "bar") {
    int xCheckStart = artist->GetBlackboard().Get<int>("SliceDFS.xCheckStart", 0);
    int ratio = (xCheckStart + 1) * 20 / 128;
    double percent = static_cast<double>(xCheckStart + 1) * 100 / 128;
    ostringstream bar;
    bar << "[" << string(ratio, '#') << string(20 - ratio, '-') << "]  "
        << fixed << setprecision(1) << percent << "%";
    artist->SendChatMessage(bar.str());
  } else if (cmd == "csafe") {
    artist->SendChatCommand(cmd);
  } else if (cmd.find("cmd") != string::npos) {
    regex pattern("cmd\\s+(.+)");
    smatch matches;
    regex_search(cmd, matches, pattern);
    string ingameCmd = matches[1];
    artist->SendChatCommand(ingameCmd);
  } else if (cmd == "name") {
    string name = artist->GetNetworkManager()->GetMyName();
    artist->SendChatMessage(name);
  } else if (cmd.find("setCol") != string::npos) {
    string name = artist->GetNetworkManager()->GetMyName();
    regex patternAssign("([^\\s]+) ([^\\s]+) (\\d+)");
    smatch matches;
    if (regex_search(cmd, matches, patternAssign)) {
      string command = matches[1];
      string exp_user = matches[2];
      int col = stoi(matches[3]);

      if (exp_user != name) return;
      Blackboard& bb = artist->GetBlackboard();
      string info = "Assign user " + exp_user + " for column " + string(matches[3]);
      cout << info << endl;
      artist->SendChatMessage(info);
      bb.Set("workCol", col);
    }
  } else if (cmd.find("setWorker") != string::npos) {
    regex patternAssign("([^\\s]+) (\\d+)");
    smatch matches;
    if (regex_search(cmd, matches, patternAssign)) {
      Blackboard& bb = artist->GetBlackboard();
      int worker_num = stoi(matches[2]);
      string info = "Setup total worker " + string(matches[2]);
      cout << info << endl;
      artist->SendChatMessage(info);
      bb.Set("workerNum", worker_num);
    }
  } else if (cmd == "checkDuty") {
    Blackboard& bb = artist->GetBlackboard();
    int workCol = bb.Get<int>("workCol", 0);
    int workerNum = bb.Get<int>("workerNum", 1);
    string info = "Max worker: " + to_string(workerNum) + ", work col: " + to_string(workCol);
    artist->SendChatMessage(info);
  } else if (cmd == "setDefault") {
    Blackboard& bb = artist->GetBlackboard();
    string info = "Reset workCol & wirkerNum value";
    artist->SendChatMessage(info);
    bb.Set("workCol", 0);
    bb.Set("workerNum", 1);
  }
}

void msgProcessor(string text, Artist *artist) {
  // regex cmdPattern("::(\\S+)"), namePattern("<([^>]+)>");
  regex cmdPattern("::(.+)"), namePattern("<([^>]+)>");

  smatch cmdMatch, nameMatch;
  string sendBy, cmd;
  if (regex_search(text, nameMatch, namePattern)) sendBy = nameMatch[1].str();
  if (regex_search(text, cmdMatch, cmdPattern)) {
    cmd = cmdMatch[1].str();
    cout << "Send by: " << sendBy << endl << "CMD: " << cmd << endl;
    cmdHandler(cmd, artist);
  }
}

Artist::Artist(const bool use_renderer, string path) : 
    SimpleBehaviourClient(use_renderer), 
    finder(shared_ptr<BehaviourClient>(this)) {
  configPath = path;
}

Artist::~Artist() {}

void Artist::Handle(ClientboundPlayerChatPacket &msg) {
  ManagersClient::Handle(msg);
  string text = msg.GetBody().GetContent();

  msgProcessor(text, this);
}

void Artist::Handle(ClientboundSystemChatPacket &msg) {
    ManagersClient::Handle(msg);

    string text = msg.GetContent().GetText();
    // Find a message from discord
    cout << text << endl;
    if (text.find("[#405home]") != string::npos) {
      msgProcessor(text, this);
    } else if (text.find("[系統] 這個領地內的區域") != string::npos) {
      SendChatMessage(text);
    }
}

void Artist::Handle(ClientboundTabListPacket &msg) {
    ManagersClient::Handle(msg);

    // LOG_INFO(endl << "Header: " << msg.GetHeader().GetText() << endl);
    // LOG_INFO(endl << "Footer: " << msg.GetFooter().GetText() << endl);
}