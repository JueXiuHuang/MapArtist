#include <iostream>
#include <string>

#include "Artist.hpp"
#include "CustomSubTree.hpp"

#include "botcraft/Game/ManagersClient.hpp"
#include "botcraft/Utilities/Logger.hpp"

using namespace Botcraft;
using namespace std;

struct Args {
    const string address = "127.0.0.1:25565";
    const string login = "33ss";
    const string configPath = "";

    int return_code = 0;
};

int main(int argc, char* argv[]) {
    try {
        // Init logging, log everything >= Info, only to console, no file
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        Logger::GetInstance().SetFilename("");
        // Add a name to this thread for logging
        Logger::GetInstance().RegisterThread("main");

        Args args;
        
        Artist client(true);

        const shared_ptr<BehaviourTree<SimpleBehaviourClient>> tree = FullTree();

        LOG_INFO("Starting connection process");
        client.Connect(args.address, args.login);
        client.SetBehaviourTree(tree);
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