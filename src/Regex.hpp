// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_REGEX_HPP_
#define SRC_REGEX_HPP_

#include <regex>

static std::regex SystemInfoPattern(R"((\[系統\])\s([^\d]+))");

static std::regex DiscordPattern(R"(\[#405home\])");

/*
Summoned to wait by CONSOLE or
Summoned to server(\d+) by CONSOLE
*/
static std::regex WaitingRoomPattern(R"(Summoned to (\w+)(\d*) by CONSOLE)");

static std::regex TpHomePattern(R"(傳送到\s(\w+)。)");

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
static std::regex FalloutTabPattern(
    R"((< 廢土伺服器 - mcFallout.net >))"
    R"([^\d]+(\d+/\d+))"
    R"([^\d]+(\d+/\d+)[^\d]+(\d+))"
    R"([^\d]+(\d+))"
    R"([^\d]+(\d+/\d+)[^\d]+(\d+)[^\d]+(\d+))"
    R"([^\d]+(\d+))"
    R"([^\d]+(\d+)[^\d]+(\d+)[^\d]+(\d+))"
    R"([^\d]+(\d+)[^\d]+(\d+:\d+\s(?:A|P)M)-[^\d-]+(-?\d+\s-?\d+\s-?\d+))"
    R"([^\d]+(\d+))"
    R"([^\d]+(\d+\s/\s\d+))");

/*
match 1:  Domain quota (remain)
match 2:  Domain quota (max)
match 3:  Domain quota from online (remain)
match 4:  Domain quota from online (max)
match 5:  Domain quota from ingot
match 6:  Domain quota from bonus
match 7:  Fly available time (second)
match 8:  Fly total time (second)
match 9:  Fly time recover 1
match 10: Fly time recover 2
match 11: Infinite fly day count
match 12: Emerald in bank
match 13: Ingot we have
match 14: Ingot price
match 15: Channel number
match 16: World (Nether, End,...)
match 17: In game time
match 18: In game position
match 19: Channel palyer count
match 20: Online player count
*/
static std::regex FalloutTabPatternVer2(
    R"(\s*§\S廢土伺服器 §\S- §\SmcFallout.net\s*\n)"
    R"(\s*\n)"
    R"(\s*§\S當前分流領地資訊§\S :\s*\n)"
    R"(\s*§\S剩餘額度§\S : §\S(\d+) §\S/§\S(\d+) 格\s*\n)"
    R"(\s*\n)"
    R"(\s*§\S數量來自 :\s*\n)"
    R"(\s*§\S線上累積§\S : §\S(\d+) §\S/(\d+) 格 §\S\+ §\S村民錠購買§\S : §\S(\d+) §\S格\s*\n)"
    R"(\s*§\S階級 §\S§\S§\S§\S§\S\S+§\S §\S §\S給予的紅利§\S : §\S(\d+) §\S格\s*\n)"
    R"(\s*§\S免費領地飛行§\S : §\S(\d+) §\S/(\d+) 秒 \(§\S不使用時每(\d+) §\S秒§\S\+(\d+) §\S秒\)\s*\n)"
    R"(\s*§\S無限領地飛行§\S : §\S(\d+) §\S 天 §\S\(使用村民錠購買\)\s*\n)"
    R"(\s*\n)"
    R"(\s*§\S綠寶石餘額§\S : §\S(\d+) §\S/ §\S村民錠餘額§\S : §\S(\d+) §\S/ §\S村民錠價格 : 每個約 §\S(\d+) 綠\s*\n)"
    R"(\s*§\S所處位置 §\S: §\S分流(\d+)§\S-§\S§\S(地獄|主世界|終界) §\S-§\S(\d+:\d+\s(?:AM|PM)) §\S-§\S座標§\S : §\S(-?\d+ -?\d+ -?\d+)\s*\n)"
    R"(\s*\n)"
    R"(\s*§\S當前分流人數 §\S: §\S§\S(\d+)\s*\n)"
    R"(\s*§\S線上人數 §\S: §\S(\d+) §\S/ §\S(\d+))");

static std::regex HungryPattern(R"(bot\s(hungry))");

/*
match 1: start
match 2: all or <user_name>
*/
static std::regex StartPattern(R"(bot\s(start)\s(\S+))");

/*
match 1: stop
match 2: all or <user_name>
*/
static std::regex StopPattern(R"(bot\s(stop)\s(\S+))");

static std::regex BarPattern(R"(bot\s(bar))");

static std::regex CsafePattern(R"(bot\s(csafe))");

/*
match 1: cmd
match 2: all or <user_name>
match 3: in game command
*/
static std::regex CmdPattern(R"(bot\s(cmd)\s(\S+)+\s(.+))");

static std::regex NamePattern(R"(bot\s(name))");

/*
match 1: assign
match 2: <user_name>
match 3: user assigned part
*/
static std::regex AssignmentPattern(R"(bot\s(assign)\s+(\S+)\s+(\d+))");

/*
match 1: worker
match 2: worker_num
*/
static std::regex WorkerPattern(R"(bot\s(worker)\s+(\d+))");

static std::regex DutyPattern(R"(bot\s(duty))");

static std::regex DefaultSettingPattern(R"(bot\s(default))");

static std::regex IngotPattern(R"(bot\s(ingot))");

static std::regex ChannelPattern(R"(bot\s(channel))");

static std::regex DetailPattern(R"(bot\s(detail))");

/*
match 1: move or bmove
match 2: x position
match 3: y position
match 4: z position
*/
static std::regex MovePattern(R"(bot\s(b?move)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+))");

static std::regex TpSuccessPattern(R"(讀取人物成功)");

#endif  // SRC_REGEX_HPP_
