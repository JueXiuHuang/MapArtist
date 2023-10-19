#include "CustomSubTree.hpp"
#include "CustomBehaviorTree.hpp"
#include "CustomTask.hpp"
#include <botcraft/AI/Tasks/AllTasks.hpp>

using namespace Botcraft;
using namespace std;

// TODO: 要把 CheckCompleteTree往後移
// 順序: init, serverload, sortchest, eat, work, complete
shared_ptr<BehaviourTree<SimpleBehaviourClient>> FullTree() {
  return Builder<SimpleBehaviourClient>("Full Tree")
    .sequence()
      .tree(InitTree())
      .leaf("Wait Server Load", WaitServerLoad)
      .leaf("Sort Chest with Desire Place", SortChestWithDesirePlace)
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
      .selector()
        .leaf("Prioritized?", CheckBlackboardBoolData, "Task.prioritized")
        .sequence()
          .leaf("Task Priortize", TaskPrioritize)
          .leaf("Dump Items", DumpItems)
          .leaf("Collect Material", CollectAllMaterial)
        .end()
      .end()
      .leaf("Task Execute scheduler", TaskExecutor)
    .end();
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> EatTree() {
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
        .selector()
          .leaf("Eat food", Eat, "minecraft:cooked_beef", true)
          .inverter().leaf("Notify", WarnConsole, "Can't eat!")
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
      .leaf("Notify", WarnConsole, "Task fully completed!")
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

shared_ptr<BehaviourTree<SimpleBehaviourClient>> BMoveTree(Position dest) {
  return Builder<SimpleBehaviourClient>("Move tree")
    .sequence()
      .leaf("move", GoTo, dest, 2, 0, 0, (4.3F), true)
      .leaf("Notify", Say, "Arrived")
      .leaf("set null tree", [](SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Status::Success; })
    .end();
}

shared_ptr<BehaviourTree<SimpleBehaviourClient>> MoveTree(Position dest) {
  return Builder<SimpleBehaviourClient>("Move tree")
    .sequence()
      .leaf("move", FindPathAndMove, dest, 2, 0, 0)
      .leaf("Notify", Say, "Arrived")
      .leaf("set null tree", [](SimpleBehaviourClient& c) { c.SetBehaviourTree(nullptr); return Status::Success; })
    .end();
}