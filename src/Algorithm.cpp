#include "Algorithm.hpp"
#include "CustomTask.hpp"
#include "Utils.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"
#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>
#include <stack>
#include <utility>

using namespace Botcraft;
using namespace std;

vector<Position> getScanOffsets(int radius) {
  vector<Position> offsets;
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      offsets.push_back(Position(x, y, 0));
    }
  }

  return offsets;
}

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
  map<string, int, MaterialCompare> itemCounter{MaterialCompare(blackboard)};

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
    const Blockstate* block = world->GetBlock(currentPos+anchor);
    if (!block) {
      // it is a air block
      if (!world->IsLoaded(currentPos+anchor)) {
        GoTo(c, currentPos+anchor, 16, 5, 5, 10);

        block = world->GetBlock(currentPos+anchor);
        if (block) block_name = block->GetName();
      }
    } else {
      block_name = block->GetName();
    }
    
    if (targetName != "minecraft:air" && block_name == "minecraft:air") {
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

  map<string, int, MaterialCompare> itemCounter{MaterialCompare(blackboard)};
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
      if(targetName == airBlockName) continue;
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
      const Blockstate* block = world->GetBlock(currentPos+anchor);
      if (!block) {
        // it is a air block
        if (!world->IsLoaded(currentPos+anchor)) {
          GoTo(c, currentPos+anchor, 16, 5, 5, 10);

          block = world->GetBlock(currentPos+anchor);
          if (block) curBlockName = block->GetName();
        }
      } else {
        curBlockName = block->GetName();
      }
      
      if (targetName != airBlockName && curBlockName == airBlockName) {
        qTaskPosition.push(currentPos+anchor);
        cout << (currentPos+anchor) << targetName << endl;
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

void SliceDFS(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position& anchor = blackboard.Get<Position>("anchor");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");
  const Position size = end - start + Position(1, 1, 1);
  vector<bool> xCheck = blackboard.Get<vector<bool>>("SliceDFS.xCheck", vector(size.x, false));

  vector<vector<vector<bool>>> visited(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));

  const int workerNum = blackboard.Get<int>("workerNum", 1);
  const int workCol = blackboard.Get<int>("workCol", 0);

  int slotCounter = 0;
  map<string, int, MaterialCompare> itemCounter{MaterialCompare(blackboard)};

  queue<Position> qTaskPosition;
  queue<string> qTaskType, qTaskName;
  stack<Position> pending;

  const vector<Position> neighbor_offsets({ Position(0, 1, 0), Position(0, -1, 0), 
                                            Position(0, 0, 1), Position(0, 0, -1)});

  for (int x = 0; x < size.x; x++) {
    if (x%workerNum != workCol) continue;
    if (xCheck[x]) continue;

    // Put every y=0 blocks into pending stack.
    for (int z = 0; z < size.z; z++) {
      const short nbtBlockId = target[x][0][z];
      const string nbtBlockName = palette.at(nbtBlockId);
      if (nbtBlockName != "minecraft:air") {
        if (z+1 < size.z) {
          const short nextNbtBlockId = target[x][0][z+1];
          const string nextNbtBlockName = palette.at(nextNbtBlockId);
          if (nextNbtBlockName == "minecraft:air") {
            pending.push(Position(x, 0, z));
            visited[x][0][z] = true;
          }
        } else {
          pending.push(Position(x, 0, z));
          visited[x][0][z] = true;
        }
      }
    }

    bool isAllDone = true;
    while (!pending.empty()) {
      Position cp = pending.top();
      pending.pop();

      const short nbtBlockId = target[cp.x][cp.y][cp.z];
      const string nbtBlockName = palette.at(nbtBlockId);
      
      string worldBlockName = GetWorldBlock(c, cp+anchor);
      string taskType = GetTaskType(worldBlockName, nbtBlockName);

      if (taskType != "None") {
        isAllDone = false;
        qTaskPosition.push(cp+anchor);
        qTaskType.push(taskType);
        qTaskName.push(nbtBlockName);

        if (taskType == "Place") {
          // maintain itemCounter and slotCounter
          if ((itemCounter[nbtBlockName]++) % 64 == 0) slotCounter++;
          if (slotCounter == 27) break;
        }
      }

      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Position newPos = cp + neighbor_offsets[i];
        bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
        bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
        bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

        if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
          short _target_id = target[newPos.x][newPos.y][newPos.z];
          string _target_name = palette.at(_target_id);
          string _worldBlockName = GetWorldBlock(c, newPos+anchor);
          if (_target_name == "minecraft:air" && _worldBlockName == "minecraft:air") continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (isAllDone) xCheck[x] = true;
    if (slotCounter == 27) break;
  }

  blackboard.Set("SliceDFS.xCheck", xCheck);
  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);
}

void SliceDFSNeighbor(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position& anchor = blackboard.Get<Position>("anchor");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");
  int xCheckStart = blackboard.Get<int>("SliceDFS.xCheckStart", 0);

  const Position size = end - start + Position(1, 1, 1);
  vector<vector<vector<bool>>> visited(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));
  vector<vector<vector<bool>>> scheduled(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));

  int slotCounter = 0;
  map<string, int, MaterialCompare> itemCounter{MaterialCompare(blackboard)};

  queue<Position> qTaskPosition;
  queue<string> qTaskType, qTaskName;
  stack<Position> pending;

  const vector<Position> dfs_offsets({Position(0, 1, 0), Position(0, -1, 0), 
                                      Position(0, 0, 1), Position(0, 0, -1)});
  const vector<Position> neighbor_offsets = getScanOffsets(2);

  for (int x = xCheckStart; x < size.x; x++) {
    // Put every y=0 blocks to pending stack
    for (int z = 0; z < size.z; z++) {
        pending.push(Position(x, 0, z));
        visited[x][0][z] = true;
    }

    bool isAllDone = true;
    while (!pending.empty()) {
      Position cp = pending.top();
      pending.pop();
      
      const short nbtBlockId = target[cp.x][cp.y][cp.z];
      const string nbtBlockName = palette.at(nbtBlockId);
      
      string worldBlockName = GetWorldBlock(c, cp+anchor);
      string taskType = GetTaskType(worldBlockName, nbtBlockName);

      if (taskType != "None" && !scheduled[cp.x][cp.y][cp.z]) {
        scheduled[cp.x][cp.y][cp.z] = true;
        isAllDone = false;
        qTaskPosition.push(cp+anchor);
        qTaskType.push(taskType);
        qTaskName.push(nbtBlockName);

        if (taskType == "Place") {
          // maintain itemCounter and slotCounter
          if ((itemCounter[nbtBlockName]++) % 64 == 0) slotCounter++;
          if (slotCounter == 27) break;
        }
      }
      
      for (auto offset : neighbor_offsets) {
        Position newPos = cp + offset;
        bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
        bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
        bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

        if (xCheck && yCheck && zCheck && !scheduled[newPos.x][newPos.y][newPos.z]) {
          string wbn = GetWorldBlock(c, newPos+anchor);
          short tid = target[newPos.x][newPos.y][newPos.z];
          string tn = palette.at(tid);
          string tt = GetTaskType(wbn, tn);

          if (tt != "None") {
            scheduled[newPos.x][newPos.y][newPos.z] = true;
            qTaskPosition.push(newPos+anchor);
            qTaskType.push(tt);
            qTaskName.push(tn);

            if (tt == "Place") {
              // maintain itemCounter and slotCounter
              if ((itemCounter[tn]++) % 64 == 0) slotCounter++;
              if (slotCounter == 27) break;
            }
          }
        }
      }

      if (slotCounter == 27) break;

      for (int i = 0; i < dfs_offsets.size(); i++) {
        Position newPos = cp + dfs_offsets[i];
        bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
        bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
        bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

        if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
          short _target_id = target[newPos.x][newPos.y][newPos.z];
          string _target_name = palette.at(_target_id);
          if (_target_name == "minecraft:air") continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (isAllDone) xCheckStart++;
    if (slotCounter == 27) break;
  }

  blackboard.Set("SliceDFS.xCheckStart", xCheckStart);
  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);
}

void SliceDFSSnake(BehaviourClient& c) {
  Blackboard& blackboard = c.GetBlackboard();
  shared_ptr<World> world = c.GetWorld();

  const Position& start = blackboard.Get<Position>("Structure.start");
  const Position& end = blackboard.Get<Position>("Structure.end");
  const Position& anchor = blackboard.Get<Position>("anchor");
  const vector<vector<vector<short>>>& target = blackboard.Get<vector<vector<vector<short>>>>("Structure.target");
  const map<short, string>& palette = blackboard.Get<map<short, string>>("Structure.palette");
  const Position size = end - start + Position(1, 1, 1);
  vector<bool> xCheck = blackboard.Get<vector<bool>>("SliceDFS.xCheck", vector(size.x, false));

  vector<vector<vector<bool>>> visited(size.x, vector<vector<bool>>(size.y, vector<bool>(size.z, false)));

  const int workerNum = blackboard.Get<int>("workerNum", 1);
  const int workCol = blackboard.Get<int>("workCol", 0);

  int slotCounter = 0;
  map<string, int, MaterialCompare> itemCounter{MaterialCompare(blackboard)};

  queue<Position> qTaskPosition;
  queue<string> qTaskType, qTaskName;
  stack<Position> pending;

  const vector<Position> neighbor_offsets({ Position(0, 1, 0), Position(0, -1, 0), 
                                            Position(0, 0, 1), Position(0, 0, -1)});

  bool inverse = false;
  for (int x = 0; x < size.x; x++) {
    if (x%workerNum != workCol) continue;
    if (xCheck[x]) continue;
    inverse = !inverse;

    // Put every y=0 blocks into pending stack.
    if (inverse) {
      for (int z = size.z-1; z > -1; z--) {
        const short nbtBlockId = target[x][0][z];
        const string nbtBlockName = palette.at(nbtBlockId);
        if (nbtBlockName != "minecraft:air") {
          if (z-1 > -1) {
            const short nextNbtBlockId = target[x][0][z-1];
            const string nextNbtBlockName = palette.at(nextNbtBlockId);
            if (nextNbtBlockName == "minecraft:air") {
              pending.push(Position(x, 0, z));
              visited[x][0][z] = true;
            }
          } else {
            pending.push(Position(x, 0, z));
            visited[x][0][z] = true;
          }
        }
      }
    } else {
      for (int z = 0; z < size.z; z++) {
        const short nbtBlockId = target[x][0][z];
        const string nbtBlockName = palette.at(nbtBlockId);
        if (nbtBlockName != "minecraft:air") {
          if (z+1 < size.z) {
            const short nextNbtBlockId = target[x][0][z+1];
            const string nextNbtBlockName = palette.at(nextNbtBlockId);
            if (nextNbtBlockName == "minecraft:air") {
              pending.push(Position(x, 0, z));
              visited[x][0][z] = true;
            }
          } else {
            pending.push(Position(x, 0, z));
            visited[x][0][z] = true;
          }
        }
      }
    }

    bool isAllDone = true;
    while (!pending.empty()) {
      Position cp = pending.top();
      pending.pop();

      const short nbtBlockId = target[cp.x][cp.y][cp.z];
      const string nbtBlockName = palette.at(nbtBlockId);
      
      string worldBlockName = GetWorldBlock(c, cp+anchor);
      string taskType = GetTaskType(worldBlockName, nbtBlockName);

      if (taskType != "None") {
        isAllDone = false;
        qTaskPosition.push(cp+anchor);
        qTaskType.push(taskType);
        qTaskName.push(nbtBlockName);

        if (taskType == "Place") {
          // maintain itemCounter and slotCounter
          if ((itemCounter[nbtBlockName]++) % 64 == 0) slotCounter++;
          if (slotCounter == 27) break;
        }
      }

      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Position newPos = cp + neighbor_offsets[i];
        bool xCheck = (newPos+anchor).x >= start.x && (newPos+anchor).x <= end.x;
        bool yCheck = (newPos+anchor).y >= start.y && (newPos+anchor).y <= end.y;
        bool zCheck = (newPos+anchor).z >= start.z && (newPos+anchor).z <= end.z;

        if (xCheck && yCheck && zCheck && !visited[newPos.x][newPos.y][newPos.z]) {
          short _target_id = target[newPos.x][newPos.y][newPos.z];
          string _target_name = palette.at(_target_id);
          string _worldBlockName = GetWorldBlock(c, newPos+anchor);
          if (_target_name == "minecraft:air" && _worldBlockName == "minecraft:air") continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (isAllDone) xCheck[x] = true;
    if (slotCounter == 27) break;
  }

  blackboard.Set("SliceDFS.xCheck", xCheck);
  blackboard.Set("qTaskPosition", qTaskPosition);
  blackboard.Set("qTaskType", qTaskType);
  blackboard.Set("qTaskName", qTaskName);
  blackboard.Set("itemCounter", itemCounter);
}