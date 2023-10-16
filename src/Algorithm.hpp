#ifndef ALGORITHM_HPP_
#define ALGORITHM_HPP_

#include "botcraft/AI/BehaviourClient.hpp"
#include "botcraft/Game/Vector3.hpp"
#include <string>

void SimpleBFS(Botcraft::BehaviourClient& c);
void SimpleDFS(Botcraft::BehaviourClient& c);
void SliceDFS(Botcraft::BehaviourClient& c);
void SliceDFSNeighbor(Botcraft::BehaviourClient& c);

// Used for material collecting optimization
struct MaterialCompare {
  Botcraft::Blackboard &bb;

  MaterialCompare(Botcraft::Blackboard &blackboard) : bb(blackboard) {}
  bool operator()(const std::string &a, const std::string &b) const {
    Botcraft::Position posA = bb.Get<std::vector<Botcraft::Position>>("chest:"+a)[0];
    Botcraft::Position posB = bb.Get<std::vector<Botcraft::Position>>("chest:"+b)[0];

    if (posA.y != posB.y) return posA.y < posB.y;
    else if (posA.z != posB.z) return posA.z < posB.z;
    else return posA.x < posB.x;
  }
};

#endif