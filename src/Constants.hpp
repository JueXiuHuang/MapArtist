// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_CONSTANTS_HPP_
#define SRC_CONSTANTS_HPP_

// dcchannel
static const char KeyDcChanID[] = "discord_channel_id";
// use.dpp
static const char KeyUseDc[] = "discord_available";
// dctoken
static const char KeyDcToken[] = "discord_token";
// anchor
static const char KeyAnchor[] = "anchor_absolute_position";
// nbt
static const char KeyNbt[] = "nbt_file_name";
// tempblock
static const char KeyTmpBlock[] = "template_block";
// prioritize
static const char KeyAlgorithm[] = "prioritize_algorithm";
// home
static const char KeyHomeCmd[] = "home_command";
// retry
static const char KeyRetry[] = "retry_times";
// neighbor
static const char KeyNeighbor[] = "check_neighbors";
// Config.loaded
static const char KeyConfigLoaded[] = "is_config_loaded";
// ExchangeRate
static const char KeyIngotRate[] = "fallout_ingot_exchange_rate";
// ChannelNumber
static const char KeyCurrChNum[] = "fallout_current_channel_number";
// CurrentPos
static const char KeyCurrPos[] = "current_position";
// Header
static const char KeyHeader[] = "fallout_header";
// workerNum
static const char KeyWorkerCount[] = "total_worker_count";
// workCol
static const char KeyWorkerCol[] = "player_assigned_column";
// GetHome
static const char KeyBotGetHome[] = "bot_get_home";
// Task.prioritized
static const char KeyTaskQueued[] = "task_already_queued";
// qTaskPosition
static const char KeyTaskPosQ[] = "task_position_queue";
// qTaskType
static const char KeyTaskTypeQ[] = "task_type_queue";
// qTaskName
static const char KeyTaskNameQ[] = "task_block_name_queue";
// itemCounter
static const char KeyItemCounter[] = "item_counter";
// Structure.start
static const char KeyNbtStart[] = "nbt_structure_start_absolute_position";
// Structure.end
static const char KeyNbtEnd[] = "nbt_structure_end_absolute_position";
// Structure.target
static const char KeyNbtTarget[] = "nbt_relative_position_and_block_id_mapping";
// Structure.palette
static const char KeyNbtPalette[] = "nbt_block_id_and_block_name_mapping";
// map_memory
static const char KeyMapMemo[] = "map_memory";
// SliceDFS.xCheck
static const char KeyXCheck[] = "x_check_array_for_slice_dfs_progress_display";
// SliceDFS.xCheckStart
static const char KeyXCheckStart[] = "magic_number_for_slice_dfs_neighbor";

#endif  // SRC_CONSTANTS_HPP_
