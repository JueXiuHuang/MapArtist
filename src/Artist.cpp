#include "Artist.hpp"
#include <iostream>
#include <string>

#include "botcraft/Game/World/World.hpp"
#include "botcraft/Game/World/Block.hpp"
#include "botcraft/Game/Entities/EntityManager.hpp"
#include "botcraft/Game/Entities/LocalPlayer.hpp"
#include "botcraft/Network/NetworkManager.hpp"

#include "botcraft/Game/Inventory/Window.hpp"

#include "botcraft/AI/BehaviourTree.hpp"
#include "botcraft/AI/Tasks/AllTasks.hpp"

using namespace Botcraft;
using namespace ProtocolCraft;

Artist::Artist(const bool use_renderer) : SimpleBehaviourClient(use_renderer) {}

Artist::~Artist() {}

void Artist::Handle(ClientboundPlayerChatPacket& msg)
{
    ManagersClient::Handle(msg);
    std::string text = msg.GetBody().GetContent();
    if (text == "!greet") {
        SendChatMessage("Hi");
    } else if (text == "!hunger") {
        Status s = IsHungry(*this, 10);
        if (s == Status::Success) {
            SendChatMessage("I'm hungry.");
        } else {
            SendChatMessage("I'm not hungry.");
        }
    }
}

void Artist::Handle(ClientboundSystemChatPacket& msg)
{
    ManagersClient::Handle(msg);

    std::cout << msg.GetContent().GetRawText() << std::endl;
}

void Artist::Handle(ClientboundTabListPacket& msg)
{
    ManagersClient::Handle(msg);

    LOG_INFO("Header: " << msg.GetHeader().GetRawText() << std::endl);
    LOG_INFO("Footer: " << msg.GetFooter().GetRawText() << std::endl);

}