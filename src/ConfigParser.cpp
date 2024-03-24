// Copyright 2024 JueXiuHuang, ldslds449

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <toml++/toml.hpp>

#include "./ConfigParser.hpp"
#include "./Utils.hpp"

using namespace std::string_view_literals;

namespace fs = std::filesystem;

std::ostream &operator<<(std::ostream &os, const ServerConf &conf) {
  os << "Address: " << conf.address << std::endl
     << "Player Name: " << conf.playerName << std::endl
     << "Online: " << (conf.online ? "True" : "False") << std::endl
     << "Reconnect: " << (conf.reconnect ? "True" : "False") << std::endl;
  return os;
}

std::ostream &operator<<(std::ostream &os, const NBTConf &conf) {
  os << "Anchor: " << conf.anchor << std::endl
     << "NBT file name: " << conf.name << std::endl
     << "Temp block: " << conf.tmpBlock << std::endl;
  return os;
}

std::ostream &operator<<(std::ostream &os, const AlgorithmConf &conf) {
  os << "Method: " << conf.method << std::endl
     << "Retry: " << conf.retry << std::endl;
  return os;
}

std::ostream &operator<<(std::ostream &os, const PrivateConf &conf) {
  os << "Enable discord: " << conf.discordEnable << std::endl
     << "Discord token: " << conf.discordToken << std::endl
     << "Discord channel: " << conf.discordChannel << std::endl;
  return os;
}

std::ostream &operator<<(std::ostream &os, const MoveConf &conf) {
  os << "Use Flash: " << (conf.use_flash ? "True" : "False") << std::endl;
  return os;
}

std::ostream &operator<<(std::ostream &os, const OtherConf &conf) {
  os << "Home command: " << conf.home << std::endl;
  return os;
}

static ServerConf parseServer(toml::table *table) {
  std::cout << "Parsing Server config..." << std::endl;
  ServerConf conf;

  conf.address = (*table)["address"].value_or(conf.address);
  conf.playerName = (*table)["playerName"].value_or(conf.playerName);
  conf.online = (*table)["online"].value_or(conf.online);
  conf.reconnect = (*table)["reconnect"].value_or(conf.reconnect);

  std::cout << conf << std::endl;

  return conf;
}

static PrivateConf parsePrivate(std::string configFileName) {
  std::cout << "Parsing private config..." << std::endl;
  toml::table tbl;
  PrivateConf conf;

  constexpr auto defaultConfig = R"(
    discord_enable = false
    discord_token = ""
    discord_channel = ""
  )"sv;

  if (configFileName == "" || !fs::exists("private.toml")) {
    std::cout << "Generate default private toml file..." << std::endl;
    tbl = toml::parse(defaultConfig);

    std::ofstream defaultFile;
    defaultFile.open("private.toml");
    defaultFile << tbl;
    defaultFile.close();
  } else if (configFileName == "") {
    std::cout << "Private config not define, read default config..."
              << std::endl;
    tbl = toml::parse(defaultConfig);
  } else {
    tbl = toml::parse_file(configFileName);
  }

  conf.discordEnable = tbl["discord_enable"].value_or(false);
  conf.discordToken = tbl["discord_token"].value_or("");
  conf.discordChannel = tbl["discord_channel"].value_or("");

  if (conf.discordToken == "" || conf.discordChannel == "") {
    conf.discordEnable = false;
  }

  std::cout << conf << std::endl;

  return conf;
}

static NBTConf parseNBT(toml::table *table) {
  std::cout << "Parsing NBT config..." << std::endl;
  NBTConf conf;

  Botcraft::Position anchor =
      ParsePositionString((*table)["anchor"].value_or("0,0,0"));
  conf.anchor = anchor;
  conf.name = (*table)["name"].value_or("");
  conf.tmpBlock = (*table)["tempblock"].value_or("");

  std::cout << conf << std::endl;

  return conf;
}

static AlgorithmConf parseAlgorithm(toml::table *table) {
  std::cout << "Parsing algorithm config..." << std::endl;
  AlgorithmConf conf;

  conf.method = (*table)["method"].value_or("");
  conf.retry = (*table)["retry"].value_or(999);

  std::cout << conf << std::endl;

  return conf;
}

static ChestConf parseChests(toml::array *materials) {
  std::cout << "Parsing chest config..." << std::endl;
  ChestConf conf;

  for (auto &material : *materials) {
    toml::table *tbl = material.as_table();
    toml::array *arr = (*tbl)["coordinate"].as_array();
    std::string name = (*tbl)["name"].value_or("");
    std::vector<Botcraft::Position> posVec;

    std::cout << name << std::endl;
    for (auto &pos : *arr) {
      std::string str = pos.value_or("0,0,0");
      posVec.push_back(ParsePositionString(str));
      std::cout << str << std::endl;
    }
    conf[name] = posVec;
  }
  return conf;
}

static MoveConf parseMove(toml::table *table) {
  std::cout << "Parsing move config..." << std::endl;
  MoveConf conf;

  conf.use_flash = (*table)["use_flash"].value_or(false);

  std::cout << conf << std::endl;

  return conf;
}

static OtherConf parseOther(toml::table *table) {
  std::cout << "Parsing other config..." << std::endl;
  OtherConf conf;

  conf.home = (*table)["home"].value_or("tp @p 0 0 0");

  std::cout << conf << std::endl;

  return conf;
}

const Config ParseConfig(std::string configFileName) {
  std::cout << "Parsing config..." << std::endl;
  Config conf;

  try {
    toml::table tbl = toml::parse_file(configFileName);

    ServerConf serverConf = parseServer(tbl["server"].as_table());
    conf.server = serverConf;

    PrivateConf privateConf = parsePrivate(tbl["private"]["name"].value_or(""));
    conf.priv = privateConf;

    NBTConf nbtConf = parseNBT(tbl["nbt"].as_table());
    conf.nbt = nbtConf;

    AlgorithmConf algoConf = parseAlgorithm(tbl["algorithm"].as_table());
    conf.algo = algoConf;

    MoveConf moveConf = parseMove(tbl["move"].as_table());
    conf.move = moveConf;

    OtherConf otherConf = parseOther(tbl["other"].as_table());
    conf.other = otherConf;

    ChestConf chestConf = parseChests(tbl["chests"].as_array());
    conf.chests = chestConf;
  } catch (const toml::parse_error &err) {
    std::cerr << "Error parsing file '" << err.source().path << "':\n"
              << err.description() << "\n (" << err.source().begin << ")\n";
  }

  return conf;
}