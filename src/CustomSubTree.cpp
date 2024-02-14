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
      .leaf("Wait Server Load", WaitServerLoad)
      .leaf("Sort Player's Inventory", SortChestWithDesirePlace)
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
    .leaf("Set null tree", [](SimpleBehaviourClient &c) {
        c.SetBehaviourTree(nullptr);
        return Status::Success;
    });
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> InitTree() {
  return Builder<SimpleBehaviourClient>("Init Tree")
    .selector()
      .leaf("Config loaded?", CheckBlackboardBoolData, "Config.loaded")
      .selector()
        .leaf("Load NBT", LoadNBT)
        // If init failed, stop the behaviour
        .tree(DisconnectTree())
      .end()
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> WorkTree() {
  return Builder<SimpleBehaviourClient>("Work Tree")
    .sequence()
      .tree(EatTree())
      .selector()
        .leaf("Prioritized?", CheckBlackboardBoolData, "Task.prioritized")
        .sequence()
          .leaf("Task Priortize", TaskPrioritize)
          .leaf("Dump Items", DumpItems)
          .leaf("Collect Material", CollectAllMaterial)
        .end()
        .tree(NullTree())
      .end()
      .leaf("Task Execute scheduler", TaskExecutor)
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
            .leaf("get food", GetFood, "minecraft:cooked_beef")
            .leaf("put in left hand", SetItemInHand, "minecraft:cooked_beef", Hand::Left)
          .end()
          .leaf("Notify", WarnConsole, "Can't find food anywhere!")
        .end()
        .leaf("Eat until full", EatUntilFull, "minecraft:cooked_beef")
        // .selector()
        //   .leaf("Eat until full", EatUntilFull, "minecraft:cooked_beef")
        //   .inverter().leaf("Notify", WarnConsole, "Can't eat!")
        //   // If we are here, hungry and can't eat --> Disconnect
        //   .tree(NullTree())
        // .end()
      .end()
    .end();
}

// TODO: review code and optimize
std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> CheckCompleteTree() {
  return Builder<SimpleBehaviourClient>("Complete check")
    .sequence()
      .leaf("check completion", CheckCompletion)
      .leaf("Notify", Say, "It's done.")
      .leaf("Notify", WarnConsole, "Task fully completed!")
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
      .leaf("Notify", Say, "Arrived")
      .leaf("set null tree", [](SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Status::Success; })
    .end();
}

std::shared_ptr<BehaviourTree<SimpleBehaviourClient>> MoveTree(Position dest) {
  return Builder<SimpleBehaviourClient>("Move tree")
    .sequence()
      .leaf("move", FindPathAndMove, dest,  0, 0, 5, 5, 0, 0,  -1, -1, -1, -1, -1, -1)
      .leaf("Notify", Say, "Arrived")
      .leaf("set null tree", [](SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Status::Success; })
    .end();
}