#ifndef CUSTOMTASK_HPP_
#define CUSTOMTASK_HPP_

#include <botcraft/AI/BehaviourClient.hpp>
#include <botcraft/AI/Status.hpp>
#include <botcraft/Game/Vector3.hpp>

/// @brief Try to get a stack (64) of food from specified location.
/// @param c The client performing the action
/// @param food_name The food item name
/// @return Success if the given food is in inventory, Failure otherwise
Botcraft::Status GetFood(Botcraft::BehaviourClient& c, const std::string& food_name);

/// @brief Just like normal SortInventory, but this will place tool at specified position.
/// @param c The client performing the action
/// @return always Success
Botcraft::Status SortChestWithDesirePlace(Botcraft::BehaviourClient& c);

/// @brief Use dfs to generate task priority queue
/// @param c The client who performs the action
/// @return Always return success
Botcraft::Status TaskPrioritize(Botcraft::BehaviourClient& c);

Botcraft::Status TaskExecutor(Botcraft::BehaviourClient& c);

Botcraft::Status DumpItems(Botcraft::BehaviourClient& c);

Botcraft::Status CollectAllMaterial(Botcraft::BehaviourClient& c);

Botcraft::Status CollectSingleMaterial(Botcraft::BehaviourClient& c, std::string itemName, int needed);

/// @brief Keep waiting until we get a valid height information
/// @param c The client performing the action
/// @return Always return success
Botcraft::Status WaitServerLoad(Botcraft::BehaviourClient& c);

/// @brief Read the blackboard NextTask.XXX values, and perform the given task
/// @param c The client performing the action
/// @return Success if the task was correctly executed, failure otherwise
Botcraft::Status ExecuteTask(Botcraft::BehaviourClient& c, std::string action, Botcraft::Position blockPos, std::string blockName);

Botcraft::Status FindPathAndMoveImpl(Botcraft::BehaviourClient&c, Botcraft::Position pos, pathfinding::goal::GoalBase<pathfinding::Position> &goal);
Botcraft::Status FindPathAndMove(Botcraft::BehaviourClient&c, Botcraft::Position pos, 
  int x_tol_pos, int x_tol_neg, int y_tol_pos, int y_tol_neg, int z_tol_pos, int z_tol_neg, 
  int excl_x_pos, int excl_x_neg, int excl_y_pos, int excl_y_neg, int excl_z_pos, int excl_z_neg);

/// @brief Check if the whole structure is built, check in the blackboard for CheckCompletion.(print_details, print_errors and full_check) to know if details should be displayed in the console
/// @param c The client performing the action
/// @return Success if the structure is fully built, failure otherwise
Botcraft::Status CheckCompletion(Botcraft::BehaviourClient& c);

/// @brief Write a message in the console, prefixed with bot name
/// @param c The client performing the action
/// @param msg The string to write in the console
/// @return Always return success
Botcraft::Status WarnConsole(Botcraft::BehaviourClient& c, const std::string& msg);

/// @brief Loads a NBT file (unzipped) and store the target structure in the blackboard of the given client
/// @param c The client performing the action
/// @return Success if the file was correctly loaded, failure otherwise
Botcraft::Status LoadNBT(Botcraft::BehaviourClient& c);

// Deprecated, config will load in Artist constructor
Botcraft::Status LoadConfig(Botcraft::BehaviourClient& c);

Botcraft::Status EatUntilFull(Botcraft::BehaviourClient& c, const std::string food);

#endif