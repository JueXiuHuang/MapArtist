// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_ARTIST_HPP_
#define SRC_ARTIST_HPP_

#include <atomic>
#include <string>

#include <botcraft/AI/SimpleBehaviourClient.hpp>

#include "./ConfigParser.hpp"
#include "./Notifier.hpp"
#include "./PathFinding.hpp"

class Artist : public Botcraft::SimpleBehaviourClient {
 public:
  PathFinder finder;
  Config conf;
  bool inWaitingRoom;
  bool waitTpFinish;
  bool hasWork;
  bool needRestart;
  Botcraft::Blackboard board;
  std::atomic<std::size_t> tpID;

  Notifier tpNotifier;

  Artist(const bool use_renderer, Config _conf);
  ~Artist();

  std::size_t getTPID();
  bool getNeedRestart();
  void setNeedRestart(const bool &restart);
  void waitTP();
  template <typename Rep, typename Period>
  bool waitTP(const std::chrono::duration<Rep, Period> &duration) {
    return tpNotifier.wait_for(duration);
  }
  template <typename Clock>
  bool waitTP(const std::chrono::time_point<Clock> &time_point) {
    return tpNotifier.wait_until(time_point);
  }

 protected:
  void Handle(ProtocolCraft::ClientboundPlayerChatPacket &msg) override;
  void Handle(ProtocolCraft::ClientboundSystemChatPacket &msg) override;
  void Handle(ProtocolCraft::ClientboundTabListPacket &msg) override;
  void Handle(ProtocolCraft::ClientboundPlayerPositionPacket &msg) override;
  void Handle(ProtocolCraft::ClientboundDisconnectPacket &msg) override;
};
#endif  // SRC_ARTIST_HPP_
