#ifndef CUSTOMSUBTREE_HPP_
#define CUSTOMSUBTREE_HPP_

#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include "botcraft/Game/Vector3.hpp"

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> FullTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> InitTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> CheckCompleteTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> DisconnectTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> EatTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> NullTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> WorkTree();

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> BMoveTree(Botcraft::Position dest);

std::shared_ptr<Botcraft::BehaviourTree<Botcraft::SimpleBehaviourClient>> MoveTree(Botcraft::Position dest);

#endif