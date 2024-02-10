#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <string>
#include <dpp/dpp.h>
#include "Artist.hpp"

class DiscordBot {
  static std::string token;
  static std::string channel;
  static Artist* artistPtr;
  dpp::cluster bot;
  std::thread dcThread;
  
  DiscordBot();
public:
  void start();
  void sendDCMessage(std::string);
  static DiscordBot& getDiscordBot();
  static void init(std::string _token, std::string _ch, Artist* _ptr);
};
#endif