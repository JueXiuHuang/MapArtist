// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_CONSTANTS_HPP_
#define SRC_CONSTANTS_HPP_

#include <string>

// Config.loaded
static const std::string KeyNBTLoaded = "is_nbt_loaded";

// ExchangeRate
static const std::string KeyIngotRate = "fallout_ingot_exchange_rate";

// ChannelNumber
static const std::string KeyCurrChNum = "fallout_current_channel_number";

// CurrentPos
static const std::string KeyCurrPos = "current_position";

// Header
static const std::string KeyHeader = "fallout_header";

// workerNum
static const std::string KeyWorkerCount = "total_worker_count";

// workCol
static const std::string KeyWorkerCol = "player_assigned_column";

// GetHome
static const std::string KeyBotGetHome = "bot_get_home";

// Task.prioritized
static const std::string KeyTaskQueued = "task_already_queued";

// qTaskPosition
static const std::string KeyTaskPosQ = "task_position_queue";

// qTaskType
static const std::string KeyTaskTypeQ = "task_type_queue";

// qTaskName
static const std::string KeyTaskNameQ = "task_block_name_queue";

// itemCounter
static const std::string KeyItemCounter = "item_counter";

// Structure.start
static const std::string KeyNbtStart = "nbt_structure_start_absolute_position";

// Structure.end
static const std::string KeyNbtEnd = "nbt_structure_end_absolute_position";

// Structure.target
static const std::string KeyNbtTarget = "nbt_relative_position_and_block_id_mapping";

// Structure.palette
static const std::string KeyNbtPalette = "nbt_block_id_and_block_name_mapping";

// map_memory
static const std::string KeyMapMemo = "map_memory";

// SliceDFS.xCheck
static const std::string KeyXCheck = "x_check_array_for_slice_dfs_progress_display";

// slice_dfs
static const std::string AlgoSliceDFS = "slice_dfs";

#endif  // SRC_CONSTANTS_HPP_
