#include "Artist.hpp"
#include "CustomSubTree.hpp"
#include "botcraft/Game/ManagersClient.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include <iostream>
#include <queue>
#include <string>

#include <Windows.h>  // must put here to avoid macro error

using namespace Botcraft;
using namespace std;

struct Args {
  // initial value
  string address = "127.0.0.1:25565";
  string login = "33ss";
  string configPath = "config_local.txt";
  bool microsoftLogin = false;
};

Args parseArgv(int argc, char* argv[]){
  queue<string> q;
  for (int i = 1; i < argc; ++i){
    q.push(string(argv[i]));
  }

  Args args;
  string help_str = string(argv[0]) + " [-a address] [-l login] [-c config] [-m] [-h]\n";
  help_str += "--address (-a): Address of server. [Default: " + args.address + "]\n";
  help_str += "--login (-l): Login user name. [Default: " + args.login + "]\n";
  help_str += "--config (-c): Path of config file. [Default: " + args.configPath + "]\n";
  help_str += "--microsoft (-m): Login with Microsoft account. [Default: " + string((args.microsoftLogin ? "True" : "False")) + "]\n";
  help_str += "--help (-h): Show help information.\n";
  
  try {
    while (!q.empty()){
      string token = q.front(); q.pop();
      if (token == "--address" || token == "-a") {
        if (q.empty() || q.front()[0] == '-') throw invalid_argument("Address value is mandatory");
        string val = q.front(); q.pop();
        args.address = val;
      } else if (token == "--login" || token == "-l") {
        if (q.empty() || q.front()[0] == '-') throw invalid_argument("Login value is mandatory");
        string val = q.front(); q.pop();
        args.login = val;
      } else if (token == "--config" || token == "-c") {
        if (q.empty() || q.front()[0] == '-') throw invalid_argument("Config value is mandatory");
        string val = q.front(); q.pop();
        args.configPath = val;
      } else if (token == "--microsoft" || token == "-m") {
        args.microsoftLogin = true;
      } else if (token == "--help" || token == "-h") {
        cout << help_str << "\n";
        exit(EXIT_SUCCESS);
      } else {
        throw invalid_argument("Unknown arguement: " + token);
      }
    }
  } catch (invalid_argument &err) {
    cerr << "ERROR: " << err.what() << "\n";
    cout << help_str << "\n";
    exit(EXIT_FAILURE);
  }
  return args;
}

int main(int argc, char* argv[]) {

#if defined(_WIN32) || defined(WIN32) 
  // Set console code page to UTF-8 so console known how to interpret string data
  SetConsoleOutputCP(CP_UTF8);
  // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
  setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

  try {
    Args args = parseArgv(argc, argv);

    // Init logging, log everything >= Info, only to console, no file
    Logger::GetInstance().SetLogLevel(LogLevel::Info);
    Logger::GetInstance().SetFilename("");
    // Add a name to this thread for logging
    Logger::GetInstance().RegisterThread("main");
    
    Artist client(true, args.configPath);

    const shared_ptr<BehaviourTree<SimpleBehaviourClient>> tree = FullTree();

    cout << "Starting connection process" << endl;
    client.Connect(args.address, args.login, args.microsoftLogin);
    client.SetAutoRespawn(true);
    client.RunBehaviourUntilClosed();
    client.Disconnect();

    return 0;
  } catch (exception &e) {
    LOG_FATAL("Exception: " << e.what());
    return 1;
  } catch (...) {
    LOG_FATAL("Unknown exception");
    return 2;
  }

  return 0;
}