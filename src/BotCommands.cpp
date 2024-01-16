#include "BotCommands.hpp"
#include "CustomSubTree.hpp"
#include "Utils.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"
#include <iomanip>

using namespace std;
using namespace Botcraft;

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

  // validate duty
  Blackboard& bb = artist->GetBlackboard();
  int workerNum = bb.Get<int>("workerNum");
  int col = bb.Get<int>("workCol");
  if (workerNum < 1 || col >= workerNum) {
    string info = "Invalid duty with workers " + to_string(workerNum) + "and col " + to_string(col);
    artist->SendChatMessage(info);
    return;
  }

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

void cmdInGameCommand(smatch matches, Artist *artist) {
  string ingameCmd = matches[3];
  string name = artist->GetNetworkManager()->GetMyName();
  string exp_user = matches[2];

  if (exp_user != name && exp_user != "all") return;
  artist->SendChatCommand(ingameCmd);
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