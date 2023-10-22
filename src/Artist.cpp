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

void cmdHandler(string cmd, Artist *artist) {
  smatch matches;
  if (regex_search(cmd, matches, HungryPattern)) {
    Status s = IsHungry(*artist, 15);

    cout << GetTime() << "Current food: " << artist->GetEntityManager()->GetLocalPlayer()->GetFood() << endl;
    if (s == Status::Success) {
      artist->SendChatMessage("I'm hungry.");
    } else {
      artist->SendChatMessage("I'm not hungry.");
    }
  } else if (regex_search(cmd, matches, StopPattern)) {
    artist->SendChatMessage("=== BOT STOP ===");
    artist->SetBehaviourTree(nullptr);
  } else if (regex_search(cmd, matches, StartPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    int workerNum = bb.Get<int>("workerNum", 1);
    int workCol = bb.Get<int>("workCol", 0);

    cout << GetTime() << "=== BOT START ===" << endl;
    artist->SendChatMessage("=== BOT START ===");
    artist->SetBehaviourTree(FullTree(), {{"configPath", artist->configPath},
                                          {"pathFinder", artist->finder}, 
                                          {"workerNum", workerNum},
                                          {"workCol", workCol}});
  } else if (regex_search(cmd, matches, BarPattern)) {
    vector<bool> xCheck = artist->GetBlackboard().Get<vector<bool>>("SliceDFS.xCheck", vector(128, false));
    int finish = 0;
    for (auto x : xCheck) {
      if (x) finish++;
    }
    int ratio = finish * 20 / 128;
    double percent = static_cast<double>(finish) * 100 / 128;

    ostringstream bar;
    bar << "[" << string(ratio, '#') << string(20 - ratio, '-') << "]  "
        << fixed << setprecision(1) << percent << "%";
    artist->SendChatMessage(bar.str());
  } else if (regex_search(cmd, matches, CsafePattern)) {
    artist->SendChatCommand(cmd);
  } else if (regex_search(cmd, matches, CmdPattern)) {
    string ingameCmd = matches[2];

    artist->SendChatCommand(ingameCmd);
  } else if (regex_search(cmd, matches, NamePattern)) {
    string name = artist->GetNetworkManager()->GetMyName();

    artist->SendChatMessage(name);
  } else if (regex_search(cmd, matches, SetColPattern)) {
    string name = artist->GetNetworkManager()->GetMyName();
    string exp_user = matches[2];
    int col = stoi(matches[3]);

    if (exp_user != name) return;
    Blackboard& bb = artist->GetBlackboard();
    string info = "Assign user " + exp_user + " for column " + string(matches[3]);

    cout << GetTime() << info << endl;
    artist->SendChatMessage(info);
    bb.Set("workCol", col);
  } else if (regex_search(cmd, matches, SetWorkerPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    int worker_num = stoi(matches[2]);
    string info = "Setup total worker " + string(matches[2]);

    cout << GetTime() << info << endl;
    artist->SendChatMessage(info);
    bb.Set("workerNum", worker_num);
  } else if (regex_search(cmd, matches, CheckDutyPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    int workCol = bb.Get<int>("workCol", 0);
    int workerNum = bb.Get<int>("workerNum", 1);
    string info = "Max worker: " + to_string(workerNum) + ", work col: " + to_string(workCol);

    artist->SendChatMessage(info);
  } else if (regex_search(cmd, matches, SetDefaultPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string info = "Reset workCol & workerNum value";

    artist->SendChatMessage(info);
    bb.Set("workCol", 0);
    bb.Set("workerNum", 1);
  } else if (regex_search(cmd, matches, IngotPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string rate = bb.Get<string>("ExchangeRate", "NOT_FOUND");
    string info = "1 Villager Ingot = " + rate + " emerald.";

    artist->SendChatMessage(info);
  } else if (regex_search(cmd, matches, ChannelPattern)) {
    Blackboard& bb = artist->GetBlackboard();
    string info = "Current channel: " + bb.Get<string>("ChannelNumber", "NOT_FOUND");

    artist->SendChatMessage(info);
  } else if (regex_search(cmd, matches, MovePattern)) {
    int x = stoi(matches[2]), y = stoi(matches[3]), z = stoi(matches[4]);
    if (matches[1] == "bmove") {
      artist->SetBehaviourTree(BMoveTree(Position(x, y, z)));
    } else {
      artist->SetBehaviourTree(MoveTree(Position(x, y, z)), {{"configPath", artist->configPath},
                                                              {"pathFinder", artist->finder}});
    }
  }
}

void msgProcessor(string text, Artist *artist) {
  // regex cmdPattern("::(\\S+)"), namePattern("<([^>]+)>");
  regex cmdPattern("::(.+)"), namePattern("<([^>]+)>");

  smatch cmdMatch, nameMatch;
  string sendBy, cmd;
  if (regex_search(text, nameMatch, namePattern)) sendBy = nameMatch[1].str();
  if (regex_search(text, cmdMatch, cmdPattern)) {
    cout << text << endl;
    cmd = cmdMatch[1].str();
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
    if (text.find("[#405home]") != string::npos) {
      msgProcessor(text, this);
    } else if (text.find("[系統] 這個領地內的區域") != string::npos) {
      SendChatMessage(text);
    }
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