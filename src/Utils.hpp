#ifndef UTILS_HPP_
#define UTILS_HPP_
#include <string>
#include <regex>
#include <botcraft/AI/BehaviourClient.hpp>
#include <botcraft/Game/Vector3.hpp>
#include <botcraft/Game/World/World.hpp>
#include "Artist.hpp"

std::string GetTime();
std::string GetWorldBlock(Botcraft::BehaviourClient& c, Botcraft::Position pos);
std::string GetTaskType(const std::string &worldBlockName, const std::string &nbtBlockName);
int GetItemAmount(Botcraft::BehaviourClient& c, std::string itemName);
Botcraft::Position ParsePositionString(std::string posStr);
void CmdHandler(std::string text, Artist *artist);
void MessageOutput(std::string text, Artist* artist);
void ListPlayerInventory(Artist* artist);

#endif