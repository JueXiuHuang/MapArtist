#include "Utils.hpp"
#include "CustomTask.hpp"
#include <iomanip>
#include <string>

using namespace std;
using namespace Botcraft;

string GetTime() {
  auto t = time(nullptr);
  auto tm = *localtime(&t);
  ostringstream oss;
  oss << "[" << put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
  return oss.str();
}

string GetWorldBlock(BehaviourClient& c, Position pos) {
  shared_ptr<World> world = c.GetWorld();

  string curBlockName = "minecraft:air";
  const Blockstate* block = world->GetBlock(pos);

  if (!block) {
    // it is a air block
    if (!world->IsLoaded(pos)) {
      FindPathAndMove(c, pos, 5, 5, 5);

      block = world->GetBlock(pos);
      if (block) curBlockName = block->GetName();
    }
  } else {
    curBlockName = block->GetName();
  }

  return curBlockName;
}

string GetTaskType(string worldBlockName, string nbtBlockName) {
  string taskType = "None";
  if (nbtBlockName != "minecraft:air" && worldBlockName == "minecraft:air") {
    taskType = "Place";
  } else if (worldBlockName != "minecraft:air" && nbtBlockName != worldBlockName) {
    taskType = "Dig";
  }

  return taskType;
}