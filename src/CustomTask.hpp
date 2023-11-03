#ifndef CUSTOMTASK_HPP_
#define CUSTOMTASK_HPP_

#include "botcraft/AI/BehaviourClient.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/Game/Vector3.hpp"

/// @brief Try to get a stack (64) of food from specified location.
/// @param c The client performing the action
/// @param food_name The food item name
/// @return Success if the given food is in inventory, Failure otherwise
Botcraft::Status GetFood(Botcraft::BehaviourClient& c, const std::string& food_name);

/// @brief Just like normal SortInventory, but this will place tool at specified position.
/// @param c The client performing the action
/// @return always Success
Botcraft::Status SortChestWithDesirePlace(Botcraft::BehaviourClient& c);

/// @brief Use bsf to generate task priority queue
/// @param c The client performing the action
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

Botcraft::Status FindPathAndMove(Botcraft::BehaviourClient&c, Botcraft::Position pos, int x_tol, int y_tol, int z_tol, int excl_x_dist = -1, int excl_y_dist = -1, int excl_z_dist = -1);

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

Botcraft::Status LoadConfig(Botcraft::BehaviourClient& c);

Botcraft::Status EatUntilFull(Botcraft::BehaviourClient& c, const std::string food);

#endif