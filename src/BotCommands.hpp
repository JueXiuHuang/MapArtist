#ifndef BOTCOMMANDS_HPP
#define BOTCOMMANDS_HPP

#include "Artist.hpp"
#include "Regex.hpp"

using namespace std;

void cmdHungry(Artist *artist);
void cmdStop(smatch matches, Artist *artist);
void cmdStart(smatch matches, Artist *artist);
void cmdBar(Artist *artist);
void cmdAssignment(smatch matches, Artist *artist);
void cmdWorker(smatch matches, Artist *artist);
void cmdDefaultSetting(Artist *artist);
void cmdMove(smatch matches, Artist *artist);
void cmdWaitingRoom(smatch matches, Artist *artist);
void cmdTpSuccess(Artist *artist);
void cmdTpHome(smatch matches, Artist *artist);

#endif