#ifndef ARTIST_HPP
#define ARTIST_HPP

#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include "PathFinding.hpp"
#include "Notifier.hpp"

class Artist : public Botcraft::SimpleBehaviourClient {
  public:
    std::string configPath;
    PathFinder finder;
    bool inWaitingRoom;
    bool waitTpFinish;
    bool hasWork;
    Botcraft::Blackboard board;

    Notifier tpNotifier;
    
    Artist(const bool use_renderer, std::string path);
    ~Artist();

    std::size_t getTPID();
    void waitTP();
    template <typename Rep, typename Period>
    bool waitTP(const std::chrono::duration<Rep, Period> &duration){ return tpNotifier.wait_for(duration); }
    template <typename Clock>
    bool waitTP(const std::chrono::time_point<Clock> &time_point){ return tpNotifier.wait_until(time_point); }

  protected:
    virtual void Handle(ProtocolCraft::ClientboundPlayerChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundSystemChatPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundTabListPacket& msg) override;
    virtual void Handle(ProtocolCraft::ClientboundPlayerPositionPacket& msg) override;
};
#endif