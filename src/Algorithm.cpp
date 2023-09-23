#include "Algorithm.hpp"

#include "queue"

#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"


using namespace Botcraft;
using namespace std;

void SimpleBFS(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position& anchor = blackboard.Get<Position>("anchor");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");

  const Position size = end - start + Position(1, 1, 1);
  vector<vector<vector<bool>>> visited(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));

  int slotCounter = 0;
  map<string, int, MaterialCompare> itemCounter;

  queue<Position> pending, qTaskPosition;
  queue<string> qTaskType, qTaskName;
  pending.push(Position(0, 0, 0));
  visited[0][0][0] = true;

  const vector<Position> neighbor_offsets({ Position(0, 1, 0), Position(0, -1, 0), 
                                            Position(0, 0, 1), Position(0, 0, -1),
                                            Position(1, 0, 0), Position(-1, 0, 0) });

  while (!pending.empty()) {
    Position currentPos = pending.front();
    pending.pop();
    
    const short current_target = target[currentPos.x][currentPos.y][currentPos.z];
    const string targetName = palette.at(current_target);

    // for blocks in pending queue, we need to check below condition:
    // 1. if this block is empty and not air, then place it. (push "Place" to qTaskType)
    // 2. if this block is not air, then check if it is the same block in nbt.
    // 2-1. skip this block if it is correct.
    // 2-2. remove this block if it is incorrect. (push "Dig" to qTaskType)
    // 3. check its neighbors.
    // 3-1 if this neighbor already visited, skip
    // 3-2 if not visited, push to pending and mark it as visited
    string block_name = "minecraft:air";
    {
      world->GetMutex().lock();
      const Block* block = world->GetBlock(currentPos+anchor);

      if (!block) {
        // it is a air block
        if (!world->IsLoaded(currentPos+anchor)) {
          world->GetMutex().unlock();
          GoTo(c, currentPos+anchor, 16, 5, 5, 10);
          world->GetMutex().lock();

          block = world->GetBlock(currentPos+anchor);
          if (block) block_name = block->GetBlockstate()->GetName();
        }
      } else {
        block_name = block->GetBlockstate()->GetName();
      }
      world->GetMutex().unlock();
    }

    if (targetName != "minecraft:air" && current_target != -1 && block_name == "minecraft:air") {
      qTaskPosition.push(currentPos+anchor);
      qTaskType.push("Place");
      qTaskName.push(targetName);

      // maintain itemCounter and slotCounter
      if ((itemCounter[targetName]++) % 64 == 0) slotCounter++;
      if (slotCounter == 27) break;
    } else if (block_name != "minecraft:air" && targetName != block_name) {
      qTaskPosition.push(currentPos+anchor);
      qTaskType.push("Dig");
      qTaskName.push(targetName);
    }

    for (int i = 0; i < neighbor_offsets.size(); i++) {
      Position newPos = currentPos + neighbor_offsets[i];
      bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
      bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
      bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

      if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
        visited[newPos.x][newPos.y][newPos.z] = true;
        pending.push(newPos);
      }
    }
  }

  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);
}