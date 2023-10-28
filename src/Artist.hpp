#ifndef ARTIST_HPP
#define ARTIST_HPP

#include "PathFinding.hpp"
#include "botcraft/AI/SimpleBehaviourClient.hpp"

class Artist : public Botcraft::SimpleBehaviourClient {
  public:
    std::string configPath;
    BotCraftFinder<> finder;
    bool inWaitingRoom;
    bool waitTpFinish;
    bool hasWork;
    std::map<std::string, std::any> backup;
    
    Artist(const bool use_renderer, std::string path);
    ~Artist();
    void Backup();
    std::map<std::string, std::any> Recover();

  protected:
    virtual void Handle(ProtocolCraft::ClientboundPlayerChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundSystemChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundTabListPacket& msg) override;
};
#endif