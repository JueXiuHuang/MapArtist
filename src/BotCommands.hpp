#ifndef BOTCOMMANDS_HPP
#define BOTCOMMANDS_HPP

#include "Artist.hpp"
#include "Regex.hpp"

void cmdHungry(Artist *artist);
void cmdStop(std::smatch matches, Artist *artist);
void cmdStart(std::smatch matches, Artist *artist);
void cmdBar(Artist *artist);
void cmdInGameCommand(std::smatch matches, Artist *artist);
void cmdAssignment(std::smatch matches, Artist *artist);
void cmdWorker(std::smatch matches, Artist *artist);
void cmdDefaultSetting(Artist *artist);
void cmdMove(std::smatch matches, Artist *artist);
void cmdWaitingRoom(std::smatch matches, Artist *artist);
void cmdTpSuccess(Artist *artist);
void cmdTpHome(std::smatch matches, Artist *artist);
void cmdDetail(std::smatch matches, Artist *artist);

#endif