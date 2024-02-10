#include <iomanip>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include "BotCommands.hpp"
#include "CustomSubTree.hpp"
#include "Utils.hpp"

using namespace Botcraft;

void cmdHungry(Artist *artist) {
  Status s = IsHungry(*artist, 15);

  std::cout << GetTime() << "Current food: " << artist->GetEntityManager()->GetLocalPlayer()->GetFood() << std::endl;
  if (s == Status::Success) {
    artist->SendChatMessage("I'm hungry.");
  } else {
    artist->SendChatMessage("I'm not hungry.");
  }
}

void cmdStop(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  if (std::string(matches[2]) != "all" && std::string(matches[2]) != name) return;
  
  artist->SendChatMessage("=== BOT STOP ===");
  artist->hasWork = false;
  artist->SetBehaviourTree(nullptr);
}

void cmdStart(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  if (std::string(matches[2]) != "all" && std::string(matches[2]) != name) return;

  // validate duty
  int workerNum = artist->board.Get<int>("workerNum", 1);
  int col = artist->board.Get<int>("workCol", 0);
  
  if (workerNum < 1 || col >= workerNum) {
    std::string info = "Invalid duty with workers " + std::to_string(workerNum) + " and col " + std::to_string(col);
    artist->SendChatMessage(info);
    return;
  }

  std::cout << GetTime() << "=== BOT START ===" << std::endl;
  artist->SendChatMessage("=== BOT START ===");
  artist->hasWork = true;
  artist->SetBehaviourTree(FullTree());
}

void cmdBar(Artist *artist) {
  std::vector<bool> xCheck = artist->board.Get<std::vector<bool>>("SliceDFS.xCheck", std::vector(128, false));
  int workers = artist->board.Get<int>("workerNum", 1);
  int col = artist->board.Get<int>("workCol", 0);
  int finish = 0;
  int duty = 0;

  for (int i = 0; i < xCheck.size(); i++) {
    if (i%workers != col) continue;
    duty++;
    if (xCheck[i]) finish++;
  }
  int ratio = finish * 20 / duty;
  double percent = static_cast<double>(finish) * 100 / duty;

  std::ostringstream bar;
  bar << "[" << std::string(ratio, '#') << std::string(20 - ratio, '-') << "]  "
      << std::fixed << std::setprecision(1) << percent << "%";
  artist->SendChatMessage(bar.str());
}

void cmdInGameCommand(std::smatch matches, Artist *artist) {
  std::string ingameCmd = matches[3];
  std::string name = artist->GetNetworkManager()->GetMyName();
  std::string exp_user = matches[2];

  if (exp_user != name && exp_user != "all") return;
  artist->SendChatCommand(ingameCmd);
}

void cmdAssignment(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  std::string exp_user = matches[2];
  int col = stoi(matches[3]);

  if (exp_user != name) return;
  std::string info = "Assign user " + exp_user + " for column " + std::string(matches[3]);

  std::cout << GetTime() << info << std::endl;
  artist->SendChatMessage(info);
  artist->board.Set("workCol", col);
}

void cmdWorker(std::smatch matches, Artist *artist) {
  int worker_num = std::stoi(matches[2]);
  std::string info = "Setup total worker " + std::string(matches[2]);

  std::cout << GetTime() << info << std::endl;
  artist->SendChatMessage(info);
  artist->board.Set("workerNum", worker_num);
}

void cmdDefaultSetting(Artist *artist) {
  Blackboard& bb = artist->GetBlackboard();
  std::string info = "Reset workCol & workerNum value";

  artist->SendChatMessage(info);
  artist->board.Set("workCol", 0);
}

void cmdMove(std::smatch matches, Artist *artist) {
  int x = std::stoi(matches[2]), y = std::stoi(matches[3]), z = std::stoi(matches[4]);

  if (matches[1] == "bmove") {
    artist->SetBehaviourTree(BMoveTree(Position(x, y, z)));
  } else {
    artist->SetBehaviourTree(MoveTree(Position(x, y, z)));
  }
}

void cmdWaitingRoom(std::smatch matches, Artist *artist) {
  if (matches[1] == "wait") {
    artist->inWaitingRoom = true;
    artist->SetBehaviourTree(nullptr);
    std::cout << GetTime() << "Transfered to waiting room, stop working..." << std::endl;
    // artist->SendChatMessage("Transfered to waiting room, stop working...");
  } else {
    if (artist->inWaitingRoom) artist->waitTpFinish = true;

    // std::string info = "Transfered to channel " + std::string(matches[1]);

    // artist->SendChatMessage(info);
  }
}

void cmdTpSuccess(Artist *artist) {
  if (artist->waitTpFinish) {
    artist->inWaitingRoom = false;
    artist->waitTpFinish = false;
    std::cout << GetTime() << "Back to work..." << std::endl;
    // artist->SendChatMessage("Back to work...");
    if (artist->hasWork) {
      artist->hasWork = true;
      artist->SetBehaviourTree(FullTree());
    }
  }
}

void cmdTpHome(std::smatch matches, Artist *artist) {
  std::string homeName = artist->board.Get<std::string>("home", "mapart");

  if (std::string(matches[1]) == homeName) {
    artist->board.Set("GetHome", true);
    std::cout << GetTime() << "Bot teleport to home." << std::endl;
  }
}