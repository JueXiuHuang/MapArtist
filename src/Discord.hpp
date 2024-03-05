// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_DISCORD_HPP_
#define SRC_DISCORD_HPP_

#include <string>

#include "./Artist.hpp"

// must be last included header
#include <dpp/cluster.h>  // NOLINT

class DiscordBot {
  static std::string token;
  static std::string channel;
  static Artist *artistPtr;
  dpp::cluster bot;
  std::thread dcThread;

  DiscordBot();

 public:
  void start();
  void sendDCMessage(std::string);
  static DiscordBot &getDiscordBot();
  static void init(std::string _token, std::string _ch, Artist *_ptr);
};
#endif  // SRC_DISCORD_HPP_
