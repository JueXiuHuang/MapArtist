#ifndef REGEX_HPP_
#define REGEX_HPP_

#include <regex>

using namespace std;

static regex SystemInfoPattern(R"((\[系統\])\s([^\d]+))");

static regex DiscordPattern(R"(\[#405home\])");

/*
Summoned to wait by CONSOLE or
Summoned to server(\d+) by CONSOLE
*/
static regex WaitingRoomPattern(R"(Summoned to (\w+)(\d*) by CONSOLE)");

static regex TpHomePattern(R"(傳送到\s(\w+)。)");

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
static regex FalloutTabPattern(R"((< 廢土伺服器 - mcFallout.net >))"
                                R"([^\d]+(\d+/\d+))"
                                R"([^\d]+(\d+/\d+)[^\d]+(\d+))"
                                R"([^\d]+(\d+))"
                                R"([^\d]+(\d+/\d+)[^\d]+(\d+)[^\d]+(\d+))"
                                R"([^\d]+(\d+))"
                                R"([^\d]+(\d+)[^\d]+(\d+)[^\d]+(\d+))"
                                R"([^\d]+(\d+)[^\d]+(\d+:\d+\s(?:A|P)M)-[^\d-]+(-?\d+\s-?\d+\s-?\d+))"
                                R"([^\d]+(\d+))"
                                R"([^\d]+(\d+\s/\s\d+))");

static regex HungryPattern(R"(bot\s(hungry))");

/*
match 1: start
match 2: all or <user_name>
*/
static regex StartPattern(R"(bot\s(start)\s(\S+))");

/*
match 1: stop
match 2: all or <user_name>
*/
static regex StopPattern(R"(bot\s(stop)\s(\S+))");

static regex BarPattern(R"(bot\s(bar))");

static regex CsafePattern(R"(bot\s(csafe))");

/*
match 1: cmd
match 2: in game command
*/
static regex CmdPattern(R"(bot\s(cmd)\s+(.+))");

static regex NamePattern(R"(bot\s(name))");

/*
match 1: assign
match 2: <user_name>
match 3: user assigned part
*/
static regex AssignmentPattern(R"(bot\s(assign)\s+(\S+)\s+(\d+))");

/*
match 1: worker
match 2: worker_num
*/
static regex WorkerPattern(R"(bot\s(worker)\s+(\d+))");

static regex DutyPattern(R"(bot\s(duty))");

static regex DefaultSettingPattern(R"(bot\s(default))");

static regex IngotPattern(R"(bot\s(ingot))");

static regex ChannelPattern(R"(bot\s(channel))");

/*
match 1: move or bmove
match 2: x position
match 3: y position
match 4: z position
*/
static regex MovePattern(R"(bot\s(b?move)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+))");

static regex TpSuccessPattern(R"(讀取人物成功)");

#endif