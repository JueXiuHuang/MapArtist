// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_CONFIGPARSER_HPP_
#define SRC_CONFIGPARSER_HPP_
#include <string>
#include <unordered_map>
#include <vector>

#include <botcraft/Game/Vector3.hpp>

using ChestConf =
    std::unordered_map<std::string, std::vector<Botcraft::Position>>;

struct NBTConf {
  Botcraft::Position anchor = Botcraft::Position(0, 0, 0);
  std::string name = "";
  std::string tmpBlock = "minecraft:cobblestone";
};

struct AlgorithmConf {
  std::string method;
  int retry;
};

struct PrivateConf {
  bool discordEnable;
  std::string discordToken;
  std::string discordChannel;
};

struct OtherConf {
  std::string home;
};

struct Config {
  NBTConf nbt;
  AlgorithmConf algo;
  PrivateConf priv;
  ChestConf chests;
  OtherConf other;
};

Config ParseConfig(std::string fileName);

#endif  // SRC_CONFIGPARSER_HPP_
