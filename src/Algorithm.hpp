#include <string>

#include "botcraft/AI/BehaviourClient.hpp"

void SimpleBFS(Botcraft::BehaviourClient& c);
void SimpleDFS(Botcraft::BehaviourClient& c);

struct MaterialCompare {
  bool operator()(const std::string& lhs, const std::string& rhs) const {
    if (lhs.find("_wool") != std::string::npos && rhs.find("_wool") != std::string::npos) {
      // If both have "_wool", sort by default
      return lhs < rhs;
    } else if (lhs.find("_wool") != std::string::npos) {
      // only left has "_wool", put at front
      return true;
    } else if (rhs.find("_wool") != std::string::npos) {
      // only right has "_wool", put at front
      return false;
    } else if (lhs.find("_terracotta") != std::string::npos && rhs.find("_terracotta") != std::string::npos) {
      // If both have "_terracotta", sort by default
      return lhs < rhs; // 如果两者都包含 "_terracotta"，按字母顺序排列
    } else if (lhs.find("_terracotta") != std::string::npos) {
      // only left has "_terracotta", put at front
      return false;
    } else if (rhs.find("_terracotta") != std::string::npos) {
      // only right has "_terracotta", put at front
      return true;
    } else if (lhs.find("_block") != std::string::npos && rhs.find("_block") != std::string::npos) {
      return lhs < rhs;
    } else if (lhs.find("_block") != std::string::npos) {
      return false;
    } else if (rhs.find("_block") != std::string::npos) {
      return true;
    } else {
      // sort by default
      return lhs < rhs;
    }
  }
};