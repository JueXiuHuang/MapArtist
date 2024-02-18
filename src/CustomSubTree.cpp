#include <botcraft/AI/Tasks/AllTasks.hpp>
#include "CustomSubTree.hpp"
#include "CustomBehaviorTree.hpp"
#include "CustomTask.hpp"

using namespace Botcraft;

// 順序: init, serverload, sortchest, complete, eat & work
std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> FullTree() {
  return Builder<SimpleBehaviourClient>("Full Tree")
    .sequence()
      .tree(InitTree())
      .leaf("wait server load", WaitServerLoad)
      .leaf("sort player's inventory", SortChestWithDesirePlace)
      .tree(EatTree())
      .selector()
        .inverter().tree(CheckCompleteTree())
        .tree(NullTree())
      .end()
      .repeater(0).sequence()
        .tree(EatTree())
        .tree(WorkTree())
      .end()
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> NullTree() {
  return Builder<SimpleBehaviourClient>("Null Tree")
    .leaf("set null tree", [](SimpleBehaviourClient &c) {
        c.SetBehaviourTree(nullptr);
        return Status::Success;
    });
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> InitTree() {
  return Builder<SimpleBehaviourClient>("Init Tree")
    .selector()
      .inverter().leaf("config loaded?", CheckArtistBlackboardBoolData, "Config.loaded")
      .selector()
        .leaf("load NBT", LoadNBT)
        // If init failed, stop the behaviour
        .tree(NullTree())
      .end()
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> WorkTree() {
  return Builder<SimpleBehaviourClient>("Work Tree")
    .sequence()
      .tree(EatTree())
      .selector()
        .leaf("prioritized?", CheckArtistBlackboardBoolData, "Task.prioritized")
        .sequence()
          .leaf("task priortize", TaskPrioritize)
          .leaf("sort inventory", SortChestWithDesirePlace)
          .leaf("dump Items", DumpItems)
          .leaf("collect Material", CollectAllMaterial)
        .end()
        .tree(NullTree())
      .end()
      .leaf("task execute scheduler", TaskExecutor)
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> EatTree() {
  return Builder<SimpleBehaviourClient>("Eat Tree")
    .selector()
      // If hungry
      .inverter().leaf("check hungry", IsHungry, 15)
      // Get some food, then eat
      .sequence()
        .selector()
          .leaf("put in left hand", SetItemInHand, "minecraft:cooked_beef", Hand::Left)
          .sequence()
            .leaf("notify", WarnConsole, "Put food in left hand fail, try to get food.")
            .leaf("get food", GetFood, "minecraft:cooked_beef")
            .leaf("put in left hand again", SetItemInHand, "minecraft:cooked_beef", Hand::Left)
          .end()
          .leaf("notify", WarnConsole, "Can't find food anywhere!")
        .end()
        .leaf("eat until full", EatUntilFull, "minecraft:cooked_beef")
      .end()
    .end();
}

// TODO: review code and optimize
std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> CheckCompleteTree() {
  return Builder<SimpleBehaviourClient>("Complete check")
    .sequence()
      .leaf("check completion", CheckCompletion)
      .leaf("notify", Say, "It's done.")
      .leaf("notify", WarnConsole, "Task fully completed!")
      .repeater(0).leaf(Yield)
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> DisconnectTree() {
  return Builder<SimpleBehaviourClient>("Disconnect")
    .sequence()
      .leaf("disconnect", Disconnect)
      .repeater(0).inverter().leaf(Yield)
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> BMoveTree(Position dest) {
  return Builder<SimpleBehaviourClient>("Move tree")
    .sequence()
      .leaf("move", GoTo, dest, 2, 0, 0, true, true, (4.3F))
      .leaf("notify", Say, "Arrived")
      .leaf("set null tree", [](SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Status::Success; })
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> MoveTree(Position dest) {
  return Builder<SimpleBehaviourClient>("Move tree")
    .sequence()
      .leaf("move", FindPathAndMove, dest,  0, 0, 5, 5, 0, 0,  -1, -1, -1, -1, -1, -1)
      .leaf("notify", Say, "Arrived")
      .leaf("set null tree", [](SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Status::Success; })
    .end();
}