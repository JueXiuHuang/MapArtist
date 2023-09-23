#ifndef ARTIST_HPP
#define ARTIST_HPP

#include "botcraft/AI/SimpleBehaviourClient.hpp"
// #include "protocolCraft/GenericHandler.hpp"

class Artist : public Botcraft::SimpleBehaviourClient {
    public:
        Artist(const bool use_renderer);
        ~Artist();

    protected:
        virtual void Handle(ProtocolCraft::ClientboundPlayerChatPacket& msg) override;
        virtual void Handle(ProtocolCraft::ClientboundSystemChatPacket& msg) override;
};
#endif