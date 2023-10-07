#ifndef ARTIST_HPP
#define ARTIST_HPP

#include "botcraft/AI/SimpleBehaviourClient.hpp"
#include "PathFinding.hpp"

class Artist : public Botcraft::SimpleBehaviourClient {
  public:
    std::string configPath;
    BotCraftFinder finder;
    Artist(const bool use_renderer, std::string path);
    ~Artist();

  protected:
    virtual void Handle(ProtocolCraft::ClientboundPlayerChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundSystemChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundTabListPacket& msg) override;
};
#endif