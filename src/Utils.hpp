#ifndef UTILS_HPP_
#define UTILS_HPP_
#include "botcraft/AI/BehaviourClient.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"
#include <string>
#include <regex>

std::string GetTime();
std::string GetWorldBlock(Botcraft::BehaviourClient& c, Botcraft::Position pos);
std::string GetTaskType(std::string worldBlockName, std::string nbtBlockName);

#endif