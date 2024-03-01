#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <string>

// dcchannel
const std::string KeyDcChanID = "discord_channel_id";
// use.dpp
const std::string KeyUseDc = "discord_available";
// dctoken
const std::string KeyDcToken = "discord_token";
// anchor
const std::string KeyAnchor = "anchor_absolute_position";
// nbt
const std::string KeyNbt = "nbt_file_name";
// tempblock
const std::string KeyTmpBlock = "template_block";
// prioritize
const std::string KeyAlgorithm = "prioritize_algorithm";
// home
const std::string KeyHomeCmd = "home_command";
// retry
const std::string KeyRetry = "retry_times";
// neighbor
const std::string KeyNeighbor = "check_neighbors";
// Config.loaded
const std::string KeyConfigLoaded = "is_config_loaded";
// ExchangeRate
const std::string KeyIngotRate = "fallout_ingot_exchange_rate";
// ChannelNumber
const std::string KeyCurrChNum = "fallout_current_channel_number";
// CurrentPos
const std::string KeyCurrPos = "current_position";
// Header
const std::string KeyHeader = "fallout_header";
// workerNum
const std::string KeyWorkerCount = "total_worker_count";
// workCol
const std::string KeyWorkerCol = "player_assigned_column";
// GetHome
const std::string KeyBotGetHome = "bot_get_home";
// Task.prioritized
const std::string KeyTaskQueued = "task_already_queued";
// qTaskPosition
const std::string KeyTaskPosQ = "task_position_queue";
// qTaskType
const std::string KeyTaskTypeQ = "task_type_queue";
// qTaskName
const std::string KeyTaskNameQ = "task_block_name_queue";
// itemCounter
const std::string KeyItemCounter = "item_counter";
// Structure.start
const std::string KeyNbtStart = "nbt_structure_start_absolute_position";
// Structure.end
const std::string KeyNbtEnd = "nbt_structure_end_absolute_position";
// Structure.target
const std::string KeyNbtTarget = "nbt_relative_position_and_block_id_mapping";
// Structure.palette
const std::string KeyNbtPalette = "nbt_block_id_and_block_name_mapping";
// map_memory
const std::string KeyMapMemo = "map_memory";
// SliceDFS.xCheck
const std::string KeyXCheck = "x_check_array_for_slice_dfs_progress_display";
// SliceDFS.xCheckStart
const std::string KeyXCheckStart = "magic_number_for_slice_dfs_neighbor";

#endif