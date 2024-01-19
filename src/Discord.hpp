#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <string>
#include <dpp/dpp.h>

class DiscordBot {
  dpp::cluster bot;

public:
  DiscordBot(std::string token);
  
  void start();
  
};
#endif