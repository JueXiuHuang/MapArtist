// Copyright 2024 JueXiuHuang, ldslds449

#include <iostream>
#include <queue>
#include <string>

#include <botcraft/Utilities/Logger.hpp>

#include "./Artist.hpp"
#include "./ConfigParser.hpp"
#include "./Constants.hpp"
#include "./CustomSubTree.hpp"
#include "./Discord.hpp"
#include "./Utils.hpp"

#ifdef _WIN32
#include <Windows.h>  // must put here to avoid macro error
#endif

struct Args {
  // initial value
  std::string configPath = "config_local.toml";
};

Args parseArgv(int argc, char *argv[]) {
  std::queue<std::string> q;
  for (int i = 1; i < argc; ++i) {
    q.push(std::string(argv[i]));
  }

  Args args;
  std::string help_str = std::string(argv[0]) + " [-c --config] [-h --help]\n";
  help_str +=
      "--config (-c): Path of config file. [Default: " + args.configPath +
      "]\n";
  help_str += "--help (-h): Show help information.\n";

  try {
    while (!q.empty()) {
      std::string token = q.front();
      q.pop();
      if (token == "--config" || token == "-c") {
        if (q.empty() || q.front()[0] == '-')
          throw std::invalid_argument("Config value is mandatory");
        std::string val = q.front();
        q.pop();
        args.configPath = val;
      } else if (token == "--help" || token == "-h") {
        std::cout << help_str << "\n";
        exit(EXIT_SUCCESS);
      } else {
        throw std::invalid_argument("Unknown arguement: " + token);
      }
    }
  } catch (std::invalid_argument &err) {
    std::cerr << "ERROR: " << err.what() << "\n";
    std::cout << help_str << "\n";
    exit(EXIT_FAILURE);
  }
  return args;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
  // Set console code page to UTF-8 so console known how to interpret string
  // data
  SetConsoleOutputCP(CP_UTF8);
  // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
  setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

  try {
    Args args = parseArgv(argc, argv);

    // Init logging, log everything >= Info, only to console, no file
    Botcraft::Logger::GetInstance().SetLogLevel(Botcraft::LogLevel::Info);
    Botcraft::Logger::GetInstance().SetFilename("");
    // Add a name to this thread for logging
    Botcraft::Logger::GetInstance().RegisterThread("main");

    const Config conf = ParseConfig(args.configPath);

    Artist client(false, conf);

    std::cout << GetTime() << "Starting discord bot" << std::endl;
    if (client.conf.priv.discordEnable) {
      std::string token = client.conf.priv.discordToken;
      std::string chan = client.conf.priv.discordChannel;
      DiscordBot::init(token, chan, &client);
      DiscordBot &b = DiscordBot::getDiscordBot();
      b.start();
    }

    std::cout << GetTime() << "Starting connection process" << std::endl;
    do {
      client.setNeedRestart(false);
      client.SetShouldBeClosed(false);
      client.Connect(conf.server.address, conf.server.playerName,
                     conf.server.online);
      client.SetAutoRespawn(true);
      if (client.hasWork) client.SetBehaviourTree(FullTree());
      client.RunBehaviourUntilClosed();
      client.Disconnect();
    } while (client.getNeedRestart() && conf.server.reconnect);

    return 0;
  } catch (std::exception &e) {
    LOG_FATAL("Exception: " << e.what());
    return 1;
  } catch (...) {
    LOG_FATAL("Unknown exception");
    return 2;
  }

  return 0;
}
