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

/*
match 1:  < 廢土伺服器 - mcFallout.net >
match 2:  Domain quota
match 3:  Domain quota from online
match 4:  Domain quota from purchase
match 5:  Domain quota from bonus
match 6:  Fly available time (second)
match 7:  Fly total time (second)
match 8:  Fly time available/total
match 9: Infinite fly day count
match 10: Emerald in bank
match 11: Currency in bank
match 12: Exchange rate
match 13: Channel number
match 14: In game time
match 15: In game position
match 16: Channel palyer count
match 17: Online player count
*/
static std::regex FalloutTabPattern("(< 廢土伺服器 - mcFallout.net >)"
                                    "[^\\d]+(\\d+/\\d+)"
                                    "[^\\d]+(\\d+/\\d+)[^\\d]+(\\d+)"
                                    "[^\\d]+(\\d+)"
                                    "[^\\d]+(\\d+/\\d+)[^\\d]+(\\d+)[^\\d]+(\\d+)"
                                    "[^\\d]+(\\d+)"
                                    "[^\\d]+(\\d+)[^\\d]+(\\d+)[^\\d]+(\\d+)"
                                    "[^\\d]+(\\d+)[^\\d]+(\\d+:\\d+\\s(?:A|P)M)-[^\\d-]+(-?\\d+\\s-?\\d+\\s-?\\d+)"
                                    "[^\\d]+(\\d+)"
                                    "[^\\d]+(\\d+\\s/\\s\\d+)");

#endif