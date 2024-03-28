// Copyright 2024 JueXiuHuang, ldslds449

#include "./BotCommands.hpp"  // NOLINT

#include <iomanip>
#include <tuple>

#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>

#include "./Constants.hpp"
#include "./CustomSubTree.hpp"
#include "./Utils.hpp"

void cmdHungry(Artist *artist) {
  Botcraft::Status s = IsHungry(*artist, 15);

  std::cout << GetTime() << "Current food: "
            << artist->GetEntityManager()->GetLocalPlayer()->GetFood()
            << std::endl;
  if (s == Botcraft::Status::Success) {
    MessageOutput("I'm hungry.", artist);
  } else {
    MessageOutput("I'm not hungry.", artist);
  }
}

void cmdStop(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  if (std::string(matches[2]) != "all" && std::string(matches[2]) != name)
    return;

  MessageOutput("=== BOT STOP ===", artist);
  artist->hasWork = false;
  artist->board.Set(KeyTaskQueued, false);
  artist->SetBehaviourTree(NullTree());
}

void cmdStart(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  if (std::string(matches[2]) != "all" && std::string(matches[2]) != name)
    return;

  // validate duty
  int workerNum = artist->board.Get<int>(KeyWorkerCount, 1);
  int col = artist->board.Get<int>(KeyWorkerCol, 0);

  if (workerNum < 1 || col >= workerNum) {
    std::string info = "Invalid duty with workers " +
                       std::to_string(workerNum) + " and col " +
                       std::to_string(col);
    MessageOutput(info, artist);
    return;
  }

  std::cout << GetTime() << "=== BOT START ===" << std::endl;
  MessageOutput("=== BOT START ===", artist);
  artist->hasWork = true;
  artist->board.Set(KeyTaskQueued, false);
  artist->SetBehaviourTree(FullTree());
}

void cmdBar(Artist *artist) {
  int ratio;
  double percent;
  std::tie(ratio, percent) = CalRatioAndPercent(artist);

  std::ostringstream bar;
  bar << "[" << std::string(ratio, '#') << std::string(20 - ratio, '-') << "]  "
      << std::fixed << std::setprecision(1) << percent << "%";
  MessageOutput(bar.str(), artist);
}

void cmdInGameCommand(std::smatch matches, Artist *artist) {
  std::string ingameCmd = matches[3];
  std::string name = artist->GetNetworkManager()->GetMyName();
  std::string exp_user = matches[2];

  if (exp_user != name && exp_user != "all") return;
  artist->SendChatCommand(ingameCmd);
  MessageOutput(ingameCmd, artist);
}

void cmdAssignment(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  std::string exp_user = matches[2];
  int col = stoi(matches[3]);

  if (exp_user != name) return;
  std::string info =
      "Assign user " + exp_user + " for column " + std::string(matches[3]);

  std::cout << GetTime() << info << std::endl;
  MessageOutput(info, artist);
  artist->board.Set(KeyWorkerCol, col);
}

void cmdWorker(std::smatch matches, Artist *artist) {
  int worker_num = std::stoi(matches[2]);
  std::string info = "Setup total worker " + std::string(matches[2]);

  std::cout << GetTime() << info << std::endl;
  MessageOutput(info, artist);
  artist->board.Set(KeyWorkerCount, worker_num);
}

void cmdDefaultSetting(Artist *artist) {
  Botcraft::Blackboard &bb = artist->GetBlackboard();
  std::string info = "Reset workCol & workerNum value";

  MessageOutput(info, artist);
  artist->board.Set(KeyWorkerCol, 0);
  artist->board.Set(KeyWorkerCount, 1);
}

void cmdMove(std::smatch matches, Artist *artist) {
  int x = std::stoi(matches[2]), y = std::stoi(matches[3]),
      z = std::stoi(matches[4]);

  if (matches[1] == "bmove") {
    artist->SetBehaviourTree(BMoveTree(Botcraft::Position(x, y, z)));
  } else {
    artist->SetBehaviourTree(MoveTree(Botcraft::Position(x, y, z)));
  }
}

void cmdWaitingRoom(std::smatch matches, Artist *artist) {
  if (matches[1] == "wait") {
    artist->inWaitingRoom = true;
    artist->SetBehaviourTree(nullptr);
    std::cout << GetTime() << "Transfered to waiting room, stop working..."
              << std::endl;
    artist->SendChatMessage("Transfered to waiting room, stop working...");
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
  std::string homeName = artist->conf.other.home;

  if (std::string(matches[1]) == homeName) {
    artist->board.Set(KeyBotGetHome, true);
    std::cout << GetTime() << "Bot teleport to home." << std::endl;
  }
}

void cmdDetail(std::smatch matches, Artist *artist) {
  int workCol = artist->board.Get<int>(KeyWorkerCol, 0);
  int workerNum = artist->board.Get<int>(KeyWorkerCount, 1);
  std::string channel =
      artist->board.Get<std::string>(KeyCurrChNum, "NOT_FOUND");
  std::string fileName = artist->conf.nbt.name;
  std::string userName = artist->GetNetworkManager()->GetMyName();

  std::ostringstream oss;
  oss << "User Name: " << userName << std::endl;
  oss << "Map Name: " << fileName << std::endl;
  oss << "Total Worker: " << workerNum << std::endl;
  oss << "Work Column: " << workCol << std::endl;
  oss << "Channel: " << channel << std::endl;

  MessageOutput(oss.str(), artist);
}

void cmdFlash(std::smatch matches, Artist *artist) {
  std::string name = artist->GetNetworkManager()->GetMyName();
  if (name != matches[2]) return;

  double x = std::stod(matches[3]);
  double y = std::stod(matches[4]);
  double z = std::stod(matches[5]);

  std::shared_ptr<Botcraft::LocalPlayer> local_player = artist->GetLocalPlayer();
  local_player->SetPosition(Botcraft::Vector3(x, y, z));
}
