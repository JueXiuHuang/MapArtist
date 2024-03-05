// Copyright 2024 JueXiuHuang, ldslds449

#include "./Algorithm.hpp"  // NOLINT

#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <botcraft/Game/Vector3.hpp>
#include <botcraft/Game/World/World.hpp>

#include "./Artist.hpp"
#include "./Constants.hpp"
#include "./CustomTask.hpp"
#include "./Utils.hpp"

std::vector<Botcraft::Position> getScanOffsets(int radius) {
  std::vector<Botcraft::Position> offsets;
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      offsets.push_back(Botcraft::Position(x, y, 0));
    }
  }

  return offsets;
}

// TODO: need to check extra block and dig it.
void SimpleBFS(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::World> world = c.GetWorld();

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const Botcraft::Position &anchor = artist.conf.nbt.anchor;
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);

  const Botcraft::Position size = end - start + Botcraft::Position(1, 1, 1);
  std::vector<std::vector<std::vector<bool>>> visited(
      size.x,
      std::vector<std::vector<bool>>(size.y, std::vector<bool>(size.z, false)));

  int slotCounter = 0;
  std::map<std::string, int, MaterialCompare> itemCounter{
      MaterialCompare(artist.conf.chests)};

  std::queue<Botcraft::Position> pending, qTaskPosition;
  std::queue<std::string> qTaskType, qTaskName;
  pending.push(Botcraft::Position(0, 0, 0));
  visited[0][0][0] = true;

  const std::vector<Botcraft::Position> neighbor_offsets(
      {Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0),
       Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1),
       Botcraft::Position(1, 0, 0), Botcraft::Position(-1, 0, 0)});

  while (!pending.empty()) {
    Botcraft::Position currentPos = pending.front();
    pending.pop();

    const int16_t current_target =
        target[currentPos.x][currentPos.y][currentPos.z];
    const std::string targetName = palette.at(current_target);

    // for blocks in pending queue, we need to check below condition:
    // 1. if this block is empty and not air, then place it. (push "Place" to
    // qTaskType)
    // 2. if this block is not air, then check if it is the same block in nbt.
    // 2-1. skip this block if it is correct.
    // 2-2. remove this block if it is incorrect. (push "Dig" to qTaskType)
    // 3. check its neighbors.
    // 3-1 if this neighbor already visited, skip
    // 3-2 if not visited, push to pending and mark it as visited
    std::string block_name = "minecraft:air";
    const Botcraft::Blockstate *block = world->GetBlock(currentPos + anchor);
    if (!block) {
      // it is a air block
      if (!world->IsLoaded(currentPos + anchor)) {
        GoTo(c, currentPos + anchor, 16, 5, 5);

        block = world->GetBlock(currentPos + anchor);
        if (block) block_name = block->GetName();
      }
    } else {
      block_name = block->GetName();
    }

    if (targetName != "minecraft:air" && block_name == "minecraft:air") {
      qTaskPosition.push(currentPos + anchor);
      qTaskType.push("Place");
      qTaskName.push(targetName);

      // maintain itemCounter and slotCounter
      if ((itemCounter[targetName]++) % 64 == 0) slotCounter++;
      if (slotCounter == 27) break;
    } else if (block_name != "minecraft:air" && targetName != block_name) {
      qTaskPosition.push(currentPos + anchor);
      qTaskType.push("Dig");
      qTaskName.push(targetName);
    }

    for (int i = 0; i < neighbor_offsets.size(); i++) {
      Botcraft::Position newPos = currentPos + neighbor_offsets[i];
      bool xCheck =
          (newPos + anchor).x >= start.x && (newPos + anchor).x <= end.x;
      bool yCheck =
          (newPos + anchor).y >= start.y && (newPos + anchor).y <= end.y;
      bool zCheck =
          (newPos + anchor).z >= start.z && (newPos + anchor).z <= end.z;

      if (xCheck && yCheck && zCheck &&
          !visited[newPos.x][newPos.y][newPos.z]) {
        visited[newPos.x][newPos.y][newPos.z] = true;
        pending.push(newPos);
      }
    }
  }

  artist.board.Set(KeyTaskPosQ, qTaskPosition);
  artist.board.Set(KeyTaskTypeQ, qTaskType);
  artist.board.Set(KeyTaskNameQ, qTaskName);
  artist.board.Set(KeyItemCounter, itemCounter);
}

void SimpleDFS(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::World> world = c.GetWorld();

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const Botcraft::Position &anchor = artist.conf.nbt.anchor;
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);

  const Botcraft::Position size = end - start + Botcraft::Position(1, 1, 1);

  std::map<std::string, int, MaterialCompare> itemCounter{
      MaterialCompare(artist.conf.chests)};
  std::queue<Botcraft::Position> pending, qTaskPosition;
  std::queue<std::string> qTaskType, qTaskName;

  int slotCounter = 0;

  std::vector<std::vector<std::vector<bool>>> visited(
      size.x,
      std::vector<std::vector<bool>>(size.y, std::vector<bool>(size.z, false)));

  const std::vector<Botcraft::Position> neighbor_offsets(
      {Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0),
       Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1),
       Botcraft::Position(1, 0, 0), Botcraft::Position(-1, 0, 0)});

  // * Candidate queue is a queue that contains placement candidates which have
  // at least one neighbour
  // * We let anchor's location be X, Y, Z

  auto distanceCMP = [](const Botcraft::Position &a,
                        const Botcraft::Position &b) {
    return a.SqrDist(Botcraft::Position(0, 0, 0)) <
           b.SqrDist(Botcraft::Position(0, 0, 0));
  };

  using pri_queue =
      std::priority_queue<Botcraft::Position, std::vector<Botcraft::Position>,
                          decltype(distanceCMP)>;
  pri_queue candidateQueue(distanceCMP);
  std::string airBlockName = "minecraft:air";

  const int xSize = target.size();
  const int ySize = (xSize > 0 ? target.front().size() : 0);
  const int zSize =
      (xSize > 0 && ySize > 0 ? target.front().front().size() : 0);
  for (int i = 0; i < xSize; ++i) {
    for (int k = 0; k < zSize; ++k) {
      const int16_t current_target = target[i][0][k];
      const std::string targetName = palette.at(current_target);
      if (targetName == airBlockName) continue;
      candidateQueue.push(Botcraft::Position(i, 0, k));
    }
  }

  while (!candidateQueue.empty() && slotCounter < 27) {
    std::stack<Botcraft::Position> searchStack;
    Botcraft::Position now = candidateQueue.top();
    searchStack.push(now);
    candidateQueue.pop();

    while (!searchStack.empty()) {
      Botcraft::Position &currentPos = searchStack.top();
      searchStack.pop();
      if (visited[currentPos.x][currentPos.y][currentPos.z]) continue;
      visited[currentPos.x][currentPos.y][currentPos.z] = true;

      // get the block name in nbt
      const int16_t current_target =
          target[currentPos.x][currentPos.y][currentPos.z];
      const std::string targetName = palette.at(current_target);

      // get the current block name
      std::string curBlockName = airBlockName;
      const Botcraft::Blockstate *block = world->GetBlock(currentPos + anchor);
      if (!block) {
        // it is a air block
        if (!world->IsLoaded(currentPos + anchor)) {
          GoTo(c, currentPos + anchor, 16, 5, 5);

          block = world->GetBlock(currentPos + anchor);
          if (block) curBlockName = block->GetName();
        }
      } else {
        curBlockName = block->GetName();
      }

      if (targetName != airBlockName && curBlockName == airBlockName) {
        qTaskPosition.push(currentPos + anchor);
        std::cout << (currentPos + anchor) << targetName << std::endl;
        qTaskType.push("Place");
        qTaskName.push(targetName);

        // maintain itemCounter and slotCounter
        if ((itemCounter[targetName]++) % 64 == 0) slotCounter++;
        if (slotCounter == 27) break;
      } else if (curBlockName != airBlockName && targetName != curBlockName) {
        qTaskPosition.push(currentPos + anchor);
        qTaskType.push("Dig");
        qTaskName.push(targetName);
      }

      // add neighbours with special order according to the distance to anchor
      std::vector<Botcraft::Position> neighbors;
      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Botcraft::Position newPos = currentPos + neighbor_offsets[i];
        bool xCheck =
            (newPos + anchor).x >= start.x && (newPos + anchor).x <= end.x;
        bool yCheck =
            (newPos + anchor).y >= start.y && (newPos + anchor).y <= end.y;
        bool zCheck =
            (newPos + anchor).z >= start.z && (newPos + anchor).z <= end.z;
        if (xCheck && yCheck && zCheck &&
            !visited[newPos.x][newPos.y][newPos.z]) {
          neighbors.push_back(newPos);
        }
      }
      sort(neighbors.begin(), neighbors.end(), distanceCMP);
      for (std::size_t j = neighbors.size() - 1; j >= 0; --j) {
        searchStack.push(neighbors[j]);
      }
    }
  }

  artist.board.Set(KeyTaskPosQ, qTaskPosition);
  artist.board.Set(KeyTaskTypeQ, qTaskType);
  artist.board.Set(KeyTaskNameQ, qTaskName);
  artist.board.Set(KeyItemCounter, itemCounter);
}

void SliceDFS(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::World> world = c.GetWorld();

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const Botcraft::Position &anchor = artist.conf.nbt.anchor;
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);
  const Botcraft::Position size = end - start + Botcraft::Position(1, 1, 1);
  std::vector<std::vector<std::vector<BlockMemo>>> mapMemory =
      artist.board.Get<std::vector<std::vector<std::vector<BlockMemo>>>>(
          KeyMapMemo);
  std::vector<bool> xCheck = artist.board.Get<std::vector<bool>>(
      KeyXCheck, std::vector(size.x, false));

  std::vector<std::vector<std::vector<bool>>> visited(
      size.x,
      std::vector<std::vector<bool>>(size.y, std::vector<bool>(size.z, false)));

  const int workerNum = artist.board.Get<int>(KeyWorkerCount, 1);
  const int workCol = artist.board.Get<int>(KeyWorkerCol, 0);

  int slotCounter = 0;
  std::map<std::string, int, MaterialCompare> itemCounter{
      MaterialCompare(artist.conf.chests)};

  std::queue<Botcraft::Position> qTaskPosition;
  std::queue<std::string> qTaskType, qTaskName;
  std::stack<Botcraft::Position> pending;

  const std::vector<Botcraft::Position> neighbor_offsets(
      {Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1),
       Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0)});

  for (int x = 0; x < size.x; x++) {
    if (x % workerNum != workCol) continue;
    if (xCheck[x]) continue;

    // Put every y=0 blocks into pending stack.
    for (int z = 0; z < size.z; z++) {
      const int16_t nbtBlockId = target[x][0][z];
      const std::string nbtBlockName = palette.at(nbtBlockId);
      if (nbtBlockName != "minecraft:air") {
        // Only add the block where top block is air
        const int16_t topNbtBlockId = target[x][1][z];
        const std::string topNbtBlockName = palette.at(topNbtBlockId);
        if (topNbtBlockName == "minecraft:air") {
          pending.push(Botcraft::Position(x, 0, z));
          visited[x][0][z] = true;
        }
      }
    }

    bool isAllDone = true;
    while (!pending.empty()) {
      Botcraft::Position cp = pending.top();
      pending.pop();

      const int16_t nbtBlockId = target[cp.x][cp.y][cp.z];
      const std::string nbtBlockName = palette.at(nbtBlockId);

      // std::string worldBlockName = GetWorldBlock(c, cp+anchor);
      std::string worldBlockName = mapMemory[cp.x][cp.y][cp.z].name;
      std::string taskType = GetTaskType(worldBlockName, nbtBlockName);

      if (taskType != "None") {
        isAllDone = false;
        qTaskPosition.push(cp + anchor);
        qTaskType.push(taskType);
        qTaskName.push(nbtBlockName);

        if (taskType == "Place") {
          // maintain itemCounter and slotCounter
          if ((itemCounter[nbtBlockName]++) % 64 == 0) slotCounter++;
          if (slotCounter == 27) break;
        }
      }

      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Botcraft::Position newPos = cp + neighbor_offsets[i];
        bool xCheck =
            (newPos + anchor).x >= start.x && (newPos + anchor).x <= end.x;
        bool yCheck =
            (newPos + anchor).y >= start.y && (newPos + anchor).y <= end.y;
        bool zCheck =
            (newPos + anchor).z >= start.z && (newPos + anchor).z <= end.z;

        if (xCheck && yCheck && zCheck &&
            !visited[newPos.x][newPos.y][newPos.z]) {
          int16_t _target_id = target[newPos.x][newPos.y][newPos.z];
          std::string _target_name = palette.at(_target_id);
          std::string _worldBlockName =
              mapMemory[newPos.x][newPos.y][newPos.z].name;
          // std::string _worldBlockName = GetWorldBlock(c, newPos+anchor);
          if (_target_name == "minecraft:air" &&
              _worldBlockName == "minecraft:air")
            continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (isAllDone) xCheck[x] = true;
    if (slotCounter == 27) break;
  }

  artist.board.Set(KeyXCheck, xCheck);
  artist.board.Set(KeyTaskPosQ, qTaskPosition);
  artist.board.Set(KeyTaskTypeQ, qTaskType);
  artist.board.Set(KeyTaskNameQ, qTaskName);
  artist.board.Set(KeyItemCounter, itemCounter);
}

void SliceDFSNeighbor(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const Botcraft::Position &anchor = artist.conf.nbt.anchor;
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);
  int xCheckStart = artist.board.Get<int>(KeyXCheckStart, 0);

  const Botcraft::Position size = end - start + Botcraft::Position(1, 1, 1);
  std::vector<std::vector<std::vector<bool>>> visited(
      size.x,
      std::vector<std::vector<bool>>(size.y, std::vector<bool>(size.z, false)));
  std::vector<std::vector<std::vector<bool>>> scheduled(
      size.x,
      std::vector<std::vector<bool>>(size.y, std::vector<bool>(size.z, false)));

  int slotCounter = 0;
  std::map<std::string, int, MaterialCompare> itemCounter{
      MaterialCompare(artist.conf.chests)};

  std::queue<Botcraft::Position> qTaskPosition;
  std::queue<std::string> qTaskType, qTaskName;
  std::stack<Botcraft::Position> pending;

  const std::vector<Botcraft::Position> dfs_offsets(
      {Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0),
       Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1)});
  const std::vector<Botcraft::Position> neighbor_offsets = getScanOffsets(2);

  for (int x = xCheckStart; x < size.x; x++) {
    // Put every y=0 blocks to pending stack
    for (int z = 0; z < size.z; z++) {
      pending.push(Botcraft::Position(x, 0, z));
      visited[x][0][z] = true;
    }

    bool isAllDone = true;
    while (!pending.empty()) {
      Botcraft::Position cp = pending.top();
      pending.pop();

      const int16_t nbtBlockId = target[cp.x][cp.y][cp.z];
      const std::string nbtBlockName = palette.at(nbtBlockId);

      std::string worldBlockName = GetWorldBlock(c, cp + anchor);
      std::string taskType = GetTaskType(worldBlockName, nbtBlockName);

      if (taskType != "None" && !scheduled[cp.x][cp.y][cp.z]) {
        scheduled[cp.x][cp.y][cp.z] = true;
        isAllDone = false;
        qTaskPosition.push(cp + anchor);
        qTaskType.push(taskType);
        qTaskName.push(nbtBlockName);

        if (taskType == "Place") {
          // maintain itemCounter and slotCounter
          if ((itemCounter[nbtBlockName]++) % 64 == 0) slotCounter++;
          if (slotCounter == 27) break;
        }
      }

      for (auto offset : neighbor_offsets) {
        Botcraft::Position newPos = cp + offset;
        bool xCheck =
            (newPos + anchor).x >= start.x && (newPos + anchor).x <= end.x;
        bool yCheck =
            (newPos + anchor).y >= start.y && (newPos + anchor).y <= end.y;
        bool zCheck =
            (newPos + anchor).z >= start.z && (newPos + anchor).z <= end.z;

        if (xCheck && yCheck && zCheck &&
            !scheduled[newPos.x][newPos.y][newPos.z]) {
          std::string wbn = GetWorldBlock(c, newPos + anchor);
          int16_t tid = target[newPos.x][newPos.y][newPos.z];
          std::string tn = palette.at(tid);
          std::string tt = GetTaskType(wbn, tn);

          if (tt != "None") {
            scheduled[newPos.x][newPos.y][newPos.z] = true;
            qTaskPosition.push(newPos + anchor);
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
        Botcraft::Position newPos = cp + dfs_offsets[i];
        bool xCheck =
            (newPos + anchor).x >= start.x && (newPos + anchor).x <= end.x;
        bool yCheck =
            (newPos + anchor).y >= start.y && (newPos + anchor).y <= end.y;
        bool zCheck =
            (newPos + anchor).z >= start.z && (newPos + anchor).z <= end.z;

        if (xCheck && yCheck && zCheck &&
            !visited[newPos.x][newPos.y][newPos.z]) {
          int16_t _target_id = target[newPos.x][newPos.y][newPos.z];
          std::string _target_name = palette.at(_target_id);
          if (_target_name == "minecraft:air") continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (isAllDone) xCheckStart++;
    if (slotCounter == 27) break;
  }

  artist.board.Set(KeyXCheckStart, xCheckStart);
  artist.board.Set(KeyTaskPosQ, qTaskPosition);
  artist.board.Set(KeyTaskTypeQ, qTaskType);
  artist.board.Set(KeyTaskNameQ, qTaskName);
  artist.board.Set(KeyItemCounter, itemCounter);
}

void SliceDFSSnake(Botcraft::BehaviourClient &c) {
  Artist &artist = static_cast<Artist &>(c);
  std::shared_ptr<Botcraft::World> world = c.GetWorld();

  const Botcraft::Position &start =
      artist.board.Get<Botcraft::Position>(KeyNbtStart);
  const Botcraft::Position &end =
      artist.board.Get<Botcraft::Position>(KeyNbtEnd);
  const Botcraft::Position &anchor = artist.conf.nbt.anchor;
  const std::vector<std::vector<std::vector<int16_t>>> &target =
      artist.board.Get<std::vector<std::vector<std::vector<int16_t>>>>(
          KeyNbtTarget);
  const std::map<int16_t, std::string> &palette =
      artist.board.Get<std::map<int16_t, std::string>>(KeyNbtPalette);
  const Botcraft::Position size = end - start + Botcraft::Position(1, 1, 1);
  std::vector<bool> xCheck = artist.board.Get<std::vector<bool>>(
      KeyXCheck, std::vector(size.x, false));

  std::vector<std::vector<std::vector<bool>>> visited(
      size.x,
      std::vector<std::vector<bool>>(size.y, std::vector<bool>(size.z, false)));

  const int workerNum = artist.board.Get<int>(KeyWorkerCount, 1);
  const int workCol = artist.board.Get<int>(KeyWorkerCol, 0);

  int slotCounter = 0;
  std::map<std::string, int, MaterialCompare> itemCounter{
      MaterialCompare(artist.conf.chests)};

  std::queue<Botcraft::Position> qTaskPosition;
  std::queue<std::string> qTaskType, qTaskName;
  std::stack<Botcraft::Position> pending;

  const std::vector<Botcraft::Position> neighbor_offsets(
      {Botcraft::Position(0, 1, 0), Botcraft::Position(0, -1, 0),
       Botcraft::Position(0, 0, 1), Botcraft::Position(0, 0, -1)});

  bool inverse = false;
  for (int x = 0; x < size.x; x++) {
    if (x % workerNum != workCol) continue;
    if (xCheck[x]) continue;
    inverse = !inverse;

    // Put every y=0 blocks into pending stack.
    if (inverse) {
      for (int z = size.z - 1; z > -1; z--) {
        const int16_t nbtBlockId = target[x][0][z];
        const std::string nbtBlockName = palette.at(nbtBlockId);
        if (nbtBlockName != "minecraft:air") {
          if (z - 1 > -1) {
            const int16_t nextNbtBlockId = target[x][0][z - 1];
            const std::string nextNbtBlockName = palette.at(nextNbtBlockId);
            if (nextNbtBlockName == "minecraft:air") {
              pending.push(Botcraft::Position(x, 0, z));
              visited[x][0][z] = true;
            }
          } else {
            pending.push(Botcraft::Position(x, 0, z));
            visited[x][0][z] = true;
          }
        }
      }
    } else {
      for (int z = 0; z < size.z; z++) {
        const int16_t nbtBlockId = target[x][0][z];
        const std::string nbtBlockName = palette.at(nbtBlockId);
        if (nbtBlockName != "minecraft:air") {
          if (z + 1 < size.z) {
            const int16_t nextNbtBlockId = target[x][0][z + 1];
            const std::string nextNbtBlockName = palette.at(nextNbtBlockId);
            if (nextNbtBlockName == "minecraft:air") {
              pending.push(Botcraft::Position(x, 0, z));
              visited[x][0][z] = true;
            }
          } else {
            pending.push(Botcraft::Position(x, 0, z));
            visited[x][0][z] = true;
          }
        }
      }
    }

    bool isAllDone = true;
    while (!pending.empty()) {
      Botcraft::Position cp = pending.top();
      pending.pop();

      const int16_t nbtBlockId = target[cp.x][cp.y][cp.z];
      const std::string nbtBlockName = palette.at(nbtBlockId);

      std::string worldBlockName = GetWorldBlock(c, cp + anchor);
      std::string taskType = GetTaskType(worldBlockName, nbtBlockName);

      if (taskType != "None") {
        isAllDone = false;
        qTaskPosition.push(cp + anchor);
        qTaskType.push(taskType);
        qTaskName.push(nbtBlockName);

        if (taskType == "Place") {
          // maintain itemCounter and slotCounter
          if ((itemCounter[nbtBlockName]++) % 64 == 0) slotCounter++;
          if (slotCounter == 27) break;
        }
      }

      for (int i = 0; i < neighbor_offsets.size(); i++) {
        Botcraft::Position newPos = cp + neighbor_offsets[i];
        bool xCheck =
            (newPos + anchor).x >= start.x && (newPos + anchor).x <= end.x;
        bool yCheck =
            (newPos + anchor).y >= start.y && (newPos + anchor).y <= end.y;
        bool zCheck =
            (newPos + anchor).z >= start.z && (newPos + anchor).z <= end.z;

        if (xCheck && yCheck && zCheck &&
            !visited[newPos.x][newPos.y][newPos.z]) {
          int16_t _target_id = target[newPos.x][newPos.y][newPos.z];
          std::string _target_name = palette.at(_target_id);
          std::string _worldBlockName = GetWorldBlock(c, newPos + anchor);
          if (_target_name == "minecraft:air" &&
              _worldBlockName == "minecraft:air")
            continue;
          visited[newPos.x][newPos.y][newPos.z] = true;
          pending.push(newPos);
        }
      }
    }

    if (isAllDone) xCheck[x] = true;
    if (slotCounter == 27) break;
  }

  artist.board.Set(KeyXCheck, xCheck);
  artist.board.Set(KeyTaskPosQ, qTaskPosition);
  artist.board.Set(KeyTaskTypeQ, qTaskType);
  artist.board.Set(KeyTaskNameQ, qTaskName);
  artist.board.Set(KeyItemCounter, itemCounter);
}
