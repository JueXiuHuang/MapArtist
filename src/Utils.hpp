#ifndef UTILS_H_
#define UTILS_H_
#include "botcraft/AI/BehaviourClient.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"
#include <string>

std::string GetTime();
std::string GetWorldBlock(Botcraft::BehaviourClient& c, Botcraft::Position pos);
std::string GetTaskType(std::string worldBlockName, std::string nbtBlockName);

#endif