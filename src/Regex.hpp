#ifndef REGEX_HPP_
#define REGEX_HPP_

#include <regex>

using namespace std;

static regex CommandPattern("::(.+)");

static regex SystemInfoPattern("(\\[系統\\])\\s([^\\d]+)");

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

static regex HungryPattern("(hungry)");

static regex StartPattern("(start)");

static regex StopPattern("(stop)");

static regex BarPattern("(bar)");

static regex CsafePattern("(csafe)");

/*
match 1: cmd
match 2: in game command
*/
static regex CmdPattern("(cmd)\\s+(.+)");

static regex NamePattern("(name)");

/*
match 1: setCol
match 2: user_name
match 3: user responsible part
*/
static regex SetColPattern("(setCol)\\s+(\\S+)\\s+(\\d+)");

/*
match 1: setWorker
match 2: worker_num
*/
static regex SetWorkerPattern("(setWorker)\\s+(\\d+)");

static regex CheckDutyPattern("(checkDuty)");

static regex SetDefaultPattern("(setDefault)");

static regex IngotPattern("(ingot)");

static regex ChannelPattern("(channel)");

/*
match 1: move or bmove
match 2: x position
match 3: y position
match 4: z position
*/
static regex MovePattern("(b?move)\\s+(-?\\d+)\\s+(-?\\d+)\\s+(-?\\d+)");

/*
Summoned to wait by CONSOLE or
Summoned to server(\d+) by CONSOLE
*/
static regex WaitingRoomPattern("Summoned to ([^\\d]+)(\\d*) by CONSOLE");

static regex TpSuccessPattern("讀取人物成功");

static regex TestAPattern("(testA)");

static regex TestBPattern("(testB)");

#endif