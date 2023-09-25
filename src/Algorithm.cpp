#include "Algorithm.hpp"

#include <queue>
#include <stack>
#include <utility>
#include <algorithm>

#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"


using namespace Botcraft;
using namespace std;

// TODO: need to check extra block and dig it.
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
          GoTo(c, currentPos+anchor, 16, 5, 5);
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

void SimpleDFS(BehaviourClient& c){
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position& anchor = blackboard.Get<Position>("anchor");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");

  const Position size = end - start + Position(1, 1, 1);

  map<string, int, MaterialCompare> itemCounter;
  queue<Position> pending, qTaskPosition;
  queue<string> qTaskType, qTaskName;

  int slotCounter = 0;

  vector<vector<vector<bool>>> visited(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));

  const vector<Position> neighbor_offsets({ Position(0, 1, 0), Position(0, -1, 0), 
                                            Position(0, 0, 1), Position(0, 0, -1),
                                            Position(1, 0, 0), Position(-1, 0, 0) });

  // * Candidate queue is a queue that contains placement candidates which have at least one neighbour
  // * We let anchor's location be X, Y, Z

  auto distanceCMP = [](const Position &a, const Position &b) { 
    return a.SqrDist(Position(0, 0, 0)) < b.SqrDist(Position(0, 0, 0)); 
  };

  using pri_queue = priority_queue<Position, vector<Position>, decltype(distanceCMP)>;
  pri_queue candidateQueue(distanceCMP);
  string airBlockName = "minecraft:air";

  const int xSize = target.size();
  const int ySize = (xSize > 0 ? target.front().size() : 0);
  const int zSize = (xSize > 0 && ySize > 0 ? target.front().front().size() : 0);
  for (int i = 0; i < xSize; ++i){
    for(int k = 0; k < zSize; ++k){
      const short current_target = target[i][0][k];
      const string targetName = palette.at(current_target);
      if(current_target == -1 || targetName == airBlockName) continue;
      candidateQueue.push(Position(i, 0, k));
    }
  }
  
  while(!candidateQueue.empty() && slotCounter < 27){
    stack<Position> searchStack;
    Position now = candidateQueue.top();
    searchStack.push(now); candidateQueue.pop();

    while (!searchStack.empty()){
      Position& currentPos = searchStack.top(); searchStack.pop();
      if (visited[currentPos.x][currentPos.y][currentPos.z]) continue;
      visited[currentPos.x][currentPos.y][currentPos.z] = true;

      // get the block name in nbt
      const short current_target = target[currentPos.x][currentPos.y][currentPos.z];
      const string targetName = palette.at(current_target);

      // get the current block name
      string curBlockName = airBlockName;
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
            if (block) curBlockName = block->GetBlockstate()->GetName();
          }
        } else {
          curBlockName = block->GetBlockstate()->GetName();
        }
        world->GetMutex().unlock();
      }
      
      if (targetName != airBlockName && current_target != -1 && curBlockName == airBlockName) {
        qTaskPosition.push(currentPos+anchor);
        LOG_INFO((currentPos+anchor) << targetName << " ");
        qTaskType.push("Place");
        qTaskName.push(targetName);

        // maintain itemCounter and slotCounter
        if ((itemCounter[targetName]++) % 64 == 0) slotCounter++;
        if (slotCounter == 27) break;
      } else if (curBlockName != airBlockName && targetName != curBlockName) {
        qTaskPosition.push(currentPos+anchor);
        qTaskType.push("Dig");
        qTaskName.push(targetName);
      }

      // add neighbours with special order according to the distance to anchor
      vector<Position> neighbors;
      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Position newPos = currentPos + neighbor_offsets[i];
        bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
        bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
        bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;
        if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
          neighbors.push_back(newPos);
        }
      }
      sort(neighbors.begin(), neighbors.end(), distanceCMP);
      for (int j = neighbors.size()-1; j >= 0; --j){
        searchStack.push(neighbors[j]);
      }
    }
  }

  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);
}

// TODO: need to check extra block and dig it.
void SliceDFS(BehaviourClient& c) {
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

  queue<Position> qTaskPosition;
  queue<string> qTaskType, qTaskName;
  stack<Position> pending;

  const vector<Position> neighbor_offsets({ Position(0, 1, 0), Position(0, -1, 0), 
                                            Position(0, 0, 1), Position(0, 0, -1)});

  for (int x = 0; x < size.x; x++) {
    for (int z = 0; z < size.z; z++) {
        pending.push(Position(x, 0, z));
        visited[x][0][z] = true;
    }

    while (!pending.empty()) {
      Position cp = pending.top();
      pending.pop();
      
      const short current_target = target[cp.x][cp.y][cp.z];
      const string targetName = palette.at(current_target);
      string block_name = "minecraft:air";
      {
        world->GetMutex().lock();
        const Block* block = world->GetBlock(cp+anchor);

        if (!block) {
          // it is a air block
          if (!world->IsLoaded(cp+anchor)) {
            world->GetMutex().unlock();
            GoTo(c, cp+anchor, 16, 5, 5);
            world->GetMutex().lock();

            block = world->GetBlock(cp+anchor);
            if (block) block_name = block->GetBlockstate()->GetName();
          }
        } else {
          block_name = block->GetBlockstate()->GetName();
        }
        world->GetMutex().unlock();
      }

      if (targetName != "minecraft:air" && current_target != -1 && block_name == "minecraft:air") {
        qTaskPosition.push(cp+anchor);
        qTaskType.push("Place");
        qTaskName.push(targetName);

        // maintain itemCounter and slotCounter
        if ((itemCounter[targetName]++) % 64 == 0) slotCounter++;
        if (slotCounter == 27) break;
      } else if (block_name != "minecraft:air" && targetName != block_name) {
        qTaskPosition.push(cp+anchor);
        qTaskType.push("Dig");
        qTaskName.push(targetName);
      }

      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Position newPos = cp + neighbor_offsets[i];
        bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
        bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
        bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

        if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
          short _target_id = target[newPos.x][newPos.y][newPos.z];
          string _target_name = palette.at(_target_id);
          if (_target_id == -1 || _target_name == "minecraft:air") continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (slotCounter == 27) break;
  }

  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);
}