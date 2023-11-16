#ifndef ARTIST_HPP
#define ARTIST_HPP

#include <future>
#include "PathFinding.hpp"
#include "Notifier.hpp"
#include "botcraft/AI/SimpleBehaviourClient.hpp"

class Artist : public Botcraft::SimpleBehaviourClient {
  public:
    std::string configPath;
    PathFinder finder;
    bool inWaitingRoom;
    bool waitTpFinish;
    bool hasWork;
    std::map<std::string, std::any> backup;

    Notifier tpNotifier;
    
    Artist(const bool use_renderer, std::string path);
    ~Artist();
    void Backup();
    std::map<std::string, std::any>& Recover();
    std::future<void> waitTP();

  protected:
    virtual void Handle(ProtocolCraft::ClientboundPlayerChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundSystemChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundTabListPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundPlayerPositionPacket& msg) override;
};
#endif