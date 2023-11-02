#ifndef REGEX_HPP_
#define REGEX_HPP_

#include <regex>

using namespace std;

static regex SystemInfoPattern("(\\[系統\\])\\s([^\\d]+)");

static regex DiscordPattern("\\[#405home\\]");

/*
Summoned to wait by CONSOLE or
Summoned to server(\d+) by CONSOLE
*/
static regex WaitingRoomPattern("Summoned to (\\w+)(\\d*) by CONSOLE");

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
static regex FalloutTabPattern("(< 廢土伺服器 - mcFallout.net >)"
                                "[^\\d]+(\\d+/\\d+)"
                                "[^\\d]+(\\d+/\\d+)[^\\d]+(\\d+)"
                                "[^\\d]+(\\d+)"
                                "[^\\d]+(\\d+/\\d+)[^\\d]+(\\d+)[^\\d]+(\\d+)"
                                "[^\\d]+(\\d+)"
                                "[^\\d]+(\\d+)[^\\d]+(\\d+)[^\\d]+(\\d+)"
                                "[^\\d]+(\\d+)[^\\d]+(\\d+:\\d+\\s(?:A|P)M)-[^\\d-]+(-?\\d+\\s-?\\d+\\s-?\\d+)"
                                "[^\\d]+(\\d+)"
                                "[^\\d]+(\\d+\\s/\\s\\d+)");

static regex HungryPattern("bot\\s(hungry)");

/*
match 1: start
match 2: all or <user_name>
*/
static regex StartPattern("bot\\s(start)\\s(\\S+)");

/*
match 1: stop
match 2: all or <user_name>
*/
static regex StopPattern("bot\\s(stop)\\s(\\S+)");

static regex BarPattern("bot\\s(bar)");

static regex CsafePattern("bot\\s(csafe)");

/*
match 1: cmd
match 2: in game command
*/
static regex CmdPattern("bot\\s(cmd)\\s+(.+)");

static regex NamePattern("bot\\s(name)");

/*
match 1: assign
match 2: <user_name>
match 3: user assigned part
*/
static regex AssignmentPattern("bot\\s(assign)\\s+(\\S+)\\s+(\\d+)");

/*
match 1: worker
match 2: worker_num
*/
static regex WorkerPattern("bot\\s(worker)\\s+(\\d+)");

static regex DutyPattern("bot\\s(duty)");

static regex DefaultSettingPattern("bot\\s(default)");

static regex IngotPattern("bot\\s(ingot)");

static regex ChannelPattern("bot\\s(channel)");

/*
match 1: move or bmove
match 2: x position
match 3: y position
match 4: z position
*/
static regex MovePattern("bot\\s(b?move)\\s+(-?\\d+)\\s+(-?\\d+)\\s+(-?\\d+)");

static regex TpSuccessPattern("讀取人物成功");

#endif