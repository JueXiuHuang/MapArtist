// Copyright 2024 JueXiuHuang, ldslds449

#include <iostream>
#include <queue>
#include <string>

#include <botcraft/Utilities/Logger.hpp>

#include "./Artist.hpp"
#include "./Constants.hpp"
#include "./Discord.hpp"
#include "./Utils.hpp"

#include <Windows.h>  // must put here to avoid macro error

struct Args {
  // initial value
  std::string address = "127.0.0.1:25565";
  std::string login = "33ss";
  std::string configPath = "config_local.txt";
  bool microsoftLogin = false;
  bool gui = false;
};

Args parseArgv(int argc, char *argv[]) {
  std::queue<std::string> q;
  for (int i = 1; i < argc; ++i) {
    q.push(std::string(argv[i]));
  }

  Args args;
  std::string help_str =
      std::string(argv[0]) +
      " [-a --address] [-l --login] [-c --config] [-m] [-h] [--gui]\n";
  help_str +=
      "--address (-a): Address of server. [Default: " + args.address + "]\n";
  help_str += "--login (-l): Login user name. [Default: " + args.login + "]\n";
  help_str +=
      "--config (-c): Path of config file. [Default: " + args.configPath +
      "]\n";
  help_str += "--microsoft (-m): Login with Microsoft account. [Default: " +
              std::string((args.microsoftLogin ? "True" : "False")) + "]\n";
  help_str += "--gui: Open GUI. [Default: " +
              std::string((args.gui ? "True" : "False")) + "]\n";
  help_str += "--help (-h): Show help information.\n";

  try {
    while (!q.empty()) {
      std::string token = q.front();
      q.pop();
      if (token == "--address" || token == "-a") {
        if (q.empty() || q.front()[0] == '-')
          throw std::invalid_argument("Address value is mandatory");
        std::string val = q.front();
        q.pop();
        args.address = val;
      } else if (token == "--login" || token == "-l") {
        if (q.empty() || q.front()[0] == '-')
          throw std::invalid_argument("Login value is mandatory");
        std::string val = q.front();
        q.pop();
        args.login = val;
      } else if (token == "--config" || token == "-c") {
        if (q.empty() || q.front()[0] == '-')
          throw std::invalid_argument("Config value is mandatory");
        std::string val = q.front();
        q.pop();
        args.configPath = val;
      } else if (token == "--microsoft" || token == "-m") {
        args.microsoftLogin = true;
      } else if (token == "--gui") {
        args.gui = true;
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
#if defined(_WIN32) || defined(WIN32)
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

    Artist client(args.gui, args.configPath);

    std::cout << GetTime() << "Starting discord bot" << std::endl;
    if (client.board.Get<bool>(KeyUseDc, false)) {
      std::string token = client.board.Get<std::string>(KeyDcToken);
      std::string chan = client.board.Get<std::string>(KeyDcChanID);
      DiscordBot::init(token, chan, &client);
      DiscordBot &b = DiscordBot::getDiscordBot();
      b.start();
    }

    std::cout << GetTime() << "Starting connection process" << std::endl;
    do {
      client.setNeedRestart(false);
      client.Connect(args.address, args.login, args.microsoftLogin);
      client.SetAutoRespawn(true);
      client.RunBehaviourUntilClosed();
    } while (client.getNeedRestart());
    client.Disconnect();

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
