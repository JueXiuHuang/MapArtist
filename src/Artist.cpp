#include "Artist.hpp"
#include "CustomSubTree.hpp"
#include "botcraft/AI/BehaviourTree.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/World/Block.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Network/NetworkManager.hpp"
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>

using namespace Botcraft;
using namespace ProtocolCraft;
using namespace std;
using json = nlohmann::json;

void cmdHandler(string cmd, Artist *artist) {
    if (cmd == "hungry") {
        Status s = IsHungry(*artist, 15);
        LOG_INFO(endl << "Current food: " << artist->GetEntityManager()->GetLocalPlayer()->GetFood());
        if (s == Status::Success) {
            artist->SendChatMessage("I'm hungry.");
        } else {
            artist->SendChatMessage("I'm not hungry.");
        }
    } else if (cmd == "stop") {
        artist->SendChatMessage("=== BOT STOP ===");
        artist->SetBehaviourTree(nullptr);
    } else if (cmd == "start") {
        artist->SendChatMessage("=== BOT START ===");
        artist->SetBehaviourTree(FullTree(), {{"configPath", artist->configPath}});
    } else if (cmd == "bar") {
        int xCheckStart = artist->GetBlackboard().Get<int>("SliceDFS.xCheckStart", 0);
        int ratio = (xCheckStart + 1) * 20 / 128;
        double percent = static_cast<double>(xCheckStart + 1) * 100 / 128;
        ostringstream bar;
        bar << "[" << string(ratio, '#') << string(20 - ratio, '-') << "]  "
            << fixed << setprecision(1) << percent << "%";
        artist->SendChatMessage(bar.str());
    }
}

void msgProcessor(string text, Artist *artist) {
    regex cmdPattern("::(\\S+)"), namePattern("<([^>]+)>");

    smatch cmdMatch, nameMatch;
    string sendBy, cmd;
    if (regex_search(text, nameMatch, namePattern)) sendBy = nameMatch[1].str();
    if (regex_search(text, cmdMatch, cmdPattern)) {
        cmd = cmdMatch[1].str();
        LOG_INFO(endl << "Send by: " << sendBy << endl << "CMD: " << cmd);
        cmdHandler(cmd, artist);
    }
}

Artist::Artist(const bool use_renderer, string path) : SimpleBehaviourClient(use_renderer) {
    configPath = path;
}

Artist::~Artist() {}

void Artist::Handle(ClientboundPlayerChatPacket &msg) {
    ManagersClient::Handle(msg);
    string text = msg.GetBody().GetContent();

    msgProcessor(text, this);
}

void Artist::Handle(ClientboundSystemChatPacket &msg) {
    ManagersClient::Handle(msg);

    string text = msg.GetContent().GetText();
    // Find a message from discord
    if (text.find("[#405home]") != string::npos) {
        LOG_INFO(endl << text);
        msgProcessor(text, this);
    }
}

void Artist::Handle(ClientboundTabListPacket &msg) {
    ManagersClient::Handle(msg);

    // LOG_INFO(endl << "Header: " << msg.GetHeader().GetText() << endl);
    // LOG_INFO(endl << "Footer: " << msg.GetFooter().GetText() << endl);
}