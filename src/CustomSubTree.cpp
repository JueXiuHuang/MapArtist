#include "CustomSubTree.hpp"

#include <botcraft/AI/Tasks/AllTasks.hpp>

#include "CustomBehaviorTree.hpp"
#include "CustomTask.hpp"

using namespace Botcraft;
using namespace std;

shared_ptr<BehaviourTree<SimpleBehaviourClient>> FullTree() {
  return Builder<SimpleBehaviourClient>("Full Tree")
    .sequence()
      .tree(InitTree())
      .leaf("Wait Server Load", WaitServerLoad)
      .leaf("Sort Chest with Desire Place", SortChestWithDesirePlace)
      .tree(EatTree())
      .tree(WorkTree())
      .selector()
        .inverter().tree(CheckCompleteTree())
        .tree(NullTree())
      .end()
    .end();
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> NullTree() {
  return Builder<SimpleBehaviourClient>("Null Tree")
    .leaf("Set null tree", [](SimpleBehaviourClient &c) {
        c.SetBehaviourTree(nullptr);
        return Status::Success;
    });
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> InitTree() {
  return Builder<SimpleBehaviourClient>("Init Tree")
    .selector()
      .leaf("Config loaded?", CheckBlackboardBoolData, "Config.loaded")
      .selector()
        .sequence()
          .leaf("Load config", LoadConfig)
          .leaf("Load NBT", LoadNBT)
        .end()
        // If init failed, stop the behaviour
        .tree(DisconnectTree())
      .end()
    .end();
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> WorkTree() {
  return Builder<SimpleBehaviourClient>("Work Tree")
    .sequence()
      .leaf("Task Priortize", TaskPriortize)
      .leaf("Dump Items", DumpItems)
      .leaf("Collect Material", CollectAllMaterial)
      .leaf("Task Execute scheduler", TaskExecutor)
    .end();
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> EatTree() {
  return Builder<SimpleBehaviourClient>("Eat Tree")
    .selector()
      // If hungry
      .inverter().leaf(IsHungry, 15)
      // Get some food, then eat
      .sequence()
        .selector()
          .leaf(SetItemInHand, "minecraft:cooked_beef", Hand::Left)
          .sequence()
            .leaf("get food", GetFood, "minecraft:cooked_beef")
            .leaf(SetItemInHand, "minecraft:cooked_beef", Hand::Left)
          .end()
          .leaf(WarnConsole, "Can't find food anywhere!")
        .end()
        .selector()
          .leaf(Eat, "minecraft:cooked_beef", true)
          .inverter().leaf(WarnConsole, "Can't eat!")
          // If we are here, hungry and can't eat --> Disconnect
          .tree(NullTree())
        .end()
      .end()
    .end();
}

// TODO: review code and optimize
shared_ptr<BehaviourTree<SimpleBehaviourClient>> CheckCompleteTree() {
  return Builder<SimpleBehaviourClient>("Complete check")
    .sequence()
      .leaf("check completion", CheckCompletion)
      .leaf("Notify", Say, "It's done.")
      .leaf(WarnConsole, "Task fully completed!")
      .repeater(0).leaf(Yield)
    .end();
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> DisconnectTree() {
  return Builder<SimpleBehaviourClient>("Disconnect")
    .sequence()
      .leaf("disconnect", Disconnect)
      .repeater(0).inverter().leaf(Yield)
    .end();
}

// TODO : review code and refactor shared_ptr<BehaviourTree<SimpleBehaviourClient>>
//        GetInventoryTree() {
//     return Builder<SimpleBehaviourClient>("List blocks in inventory")
//         // List all blocks in the inventory
//         .selector()
//             .leaf("get block in inventory", GetBlocksAvailableInInventory)
//             // If no block found, get some in neighbouring chests
//             .sequence()
//                 .selector()
//                     .leaf("get some blocks from chests", SwapChestsInventory,
//                     "minecraft:cooked_beef", true)
//                     .inverter().leaf(WarnConsole, "Can't swap with chests,
//                     will wait before retrying.")
//                     // If the previous task failed, maybe chests were just
//                     // not loaded yet, sleep for ~1 second
//                     .inverter().repeater(100).leaf(Yield)
//                 .end()
//                 .selector()
//                     .leaf("get block in inventory after swapping",
//                     GetBlocksAvailableInInventory)
//                     .inverter().leaf(WarnConsole, "No more block in chests, I
//                     will stop here.") .tree(DisconnectTree())
//                 .end()
//             .end()
//         .end();
// }

// TODO: review code and refactor
// shared_ptr<BehaviourTree<SimpleBehaviourClient>> PlaceBlockTree() {
//     return Builder<SimpleBehaviourClient>("Place block")
//         .selector()
//             // Try to perform a task 5 times
//             .decorator<RepeatUntilSuccess<SimpleBehaviourClient>>(5).selector()
//                 .sequence()
//                     .leaf("find next task", FindNextTask)
//                     .leaf("execute next task", ExecuteNextTask)
//                 .end()
//                 // If the previous task failed, sleep for ~1 second
//                 // before retrying to get an action
//                 .inverter().repeater(100).leaf(Yield)
//             .end()
//             // If failed 5 times, put all blocks in chests to
//             // randomize available blocks for next time
//             .leaf("dump all items in chest", SwapChestsInventory,
//             "minecraft:cooked_beef", false)
//         .end();
// }