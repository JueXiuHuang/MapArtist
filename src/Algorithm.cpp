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
