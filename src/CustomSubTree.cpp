// Copyright 2024 JueXiuHuang, ldslds449

#include "./CustomSubTree.hpp"  // NOLINT

#include <botcraft/AI/Tasks/AllTasks.hpp>

#include "./CustomBehaviorTree.hpp"
#include "./CustomTask.hpp"
#include "./Constants.hpp"

// Order: init, serverload, sortchest, complete, eat & work
std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> FullTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Full Tree")
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

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> NullTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Null Tree")
    .leaf("set null tree", [](Botcraft::SimpleBehaviourClient &c) {
        c.SetBehaviourTree(nullptr);
        return Botcraft::Status::Success;
    });
}

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> InitTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Init Tree")
    .selector()
      .leaf("nbt file loaded?", CheckArtistBlackboardBoolData, KeyNBTLoaded)
      .selector()
        .leaf("load NBT", LoadNBT)
        // If init failed, stop the behaviour
        .tree(NullTree())
      .end()
    .end();
}

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> WorkTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Work Tree")
    .sequence()
      .tree(EatTree())
      .selector()
        .leaf("prioritized?", CheckArtistBlackboardBoolData, KeyTaskQueued)
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

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> EatTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Eat Tree")
    .selector()
      // If hungry
      .inverter().leaf("check hungry", Botcraft::IsHungry, 15)
      // Get some food, then eat
      .sequence()
        .selector()
          .leaf("put in left hand", Botcraft::SetItemInHand, "minecraft:cooked_beef", Botcraft::Hand::Left)
          .sequence()
            .leaf("notify", WarnConsole, "Put food in left hand fail, try to get food.")
            .leaf("get food", GetFood, "minecraft:cooked_beef")
            .leaf("put in left hand again", Botcraft::SetItemInHand, "minecraft:cooked_beef", Botcraft::Hand::Left)
          .end()
          .leaf("notify", WarnConsole, "Can't find food anywhere!")
        .end()
        .leaf("eat until full", EatUntilFull, "minecraft:cooked_beef")
      .end()
    .end();
}

// TODO review code and optimize
std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> CheckCompleteTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Complete check")
    .sequence()
      .leaf("check completion", CheckCompletion)
      .leaf("notify", Botcraft::Say, "It's done.")
      .leaf("notify", WarnConsole, "Task fully completed!")
      .repeater(0).leaf(Botcraft::Yield)
    .end();
}

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> DisconnectTree() {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Disconnect")
    .sequence()
      .leaf("disconnect", Botcraft::Disconnect)
      .repeater(0).inverter().leaf(Botcraft::Yield)
    .end();
}

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> BMoveTree(Botcraft::Position dest) {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Move tree")
      .sequence()
      .leaf("move", Botcraft::GoTo, dest, 5, 0, 0, true, true, 1.0)
      .leaf("notify", Botcraft::Say, "Arrived")
      .leaf("set null tree",
            [](Botcraft::SimpleBehaviourClient &c) {
              c.SetBehaviourTree(nullptr);
              return Botcraft::Status::Success;
            })
      .end();
}

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> MoveTree(Botcraft::Position dest) {
  return Botcraft::Builder<Botcraft::SimpleBehaviourClient>("Move tree")
      .sequence()
      .leaf("move", FindPathAndMove, dest, 0, 0, 5, 5, 0, 0, -1, -1, -1, -1, -1,
            -1)
      .leaf("notify", Botcraft::Say, "Arrived")
      .leaf("set null tree",
            [](Botcraft::SimpleBehaviourClient &c) {
              c.SetBehaviourTree(nullptr);
              return Botcraft::Status::Success;
            })
      .end();
}
