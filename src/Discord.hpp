#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <string>
#include <dpp/dpp.h>

class DiscordBot {
  static DiscordBot ref;
  static std::string token;
  dpp::cluster bot;

  DiscordBot();
public:
  void start();
  static DiscordBot& getDiscordBot();
  static void init(std::string _token);
};
#endif