// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_DS_OCTREE_HPP_
#define SRC_DS_OCTREE_HPP_

#include <array>
#include <iostream>
#include <memory>
#include <stack>
#include <tuple>
#include <utility>

#include "./Point3D.hpp"

#define CHILD_NUM 8
#define CENTER(minCorner, maxCorner)             \
  (ds::Point3D{(maxCorner.x + minCorner.x) >> 1, \
               (maxCorner.y + minCorner.y) >> 1, \
               (maxCorner.z + minCorner.z) >> 1})

namespace ds {

// Octree implementation
// not thread-safe
template <class T>
class Octree {
 public:
  Octree(const Point3D &_minCorner, const Point3D &_maxCorner,
         const T &_defaultValue)
      : root(std::make_shared<OcNode>(_minCorner, _maxCorner, true,
                                      _defaultValue)) {}

  void update(const Point3D &p, const T &data) {
    // check input
    if (!in(p)) {
      std::cerr << "Octree::update Invalid Point (" << p.x << "," << p.y << ","
                << p.z << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    // find leaf
    std::stack<std::shared_ptr<OcNode>> path;
    std::shared_ptr<OcNode> now = root;
    while (now && !now->isLeaf) {
      path.push(now);
      now = now->children[now->getID(p)];
    }
    assert(now);

    // check whether we need to update
    if (*now->data != data) {
      // divide until there is only one point
      bool hasDivided = false;
      while (!now->onlyContain1Point()) {
        now->divide();
        uint8_t id = now->getID(p);
        now = now->children[id];
        hasDivided = true;
        node_count += CHILD_NUM;
      }
      // update data
      now->data = std::make_unique<T>(data);
      // check merge value
      if (!hasDivided) {
        while (!path.empty() && path.top()->merge()) {
          path.pop();
          node_count -= CHILD_NUM;
        }
      }
    }
  }

  T get(const Point3D &p) const {
    // check input
    if (!in(p)) {
      std::cerr << "Octree::get Invalid Point (" << p.x << "," << p.y << ","
                << p.z << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    // find leaf
    std::shared_ptr<OcNode> now = root;
    while (now && !now->isLeaf) {
      now = now->children[now->getID(p)];
    }
    assert(now);

    return *now->data;
  }

  inline bool in(const Point3D &p) const {
    return root->minCorner <= p && p < root->maxCorner;
  }

  inline std::size_t getNodeCount() const { return node_count; }

 private:
  struct OcNode {
    bool isLeaf;
    std::unique_ptr<T> data;
    const Point3D minCorner, maxCorner;
    std::array<std::shared_ptr<OcNode>, CHILD_NUM> children;

    OcNode(const Point3D &_minCorner, const Point3D &_maxCorner,
           const bool &_isLeaf, const T &_data)
        : minCorner(_minCorner),
          maxCorner(_maxCorner),
          isLeaf(_isLeaf),
          data(std::make_unique<T>(_data)) {}

    uint8_t getID(const Point3D &p) {
      auto center = CENTER(minCorner, maxCorner);
      uint8_t id = 0;
      id |= (p.x >= center.x) << 0;
      id |= (p.y >= center.y) << 1;
      id |= (p.z >= center.z) << 2;
      return id;
    }

    std::tuple<Point3D, Point3D> getNewRect(const uint8_t &id) {
      auto center = CENTER(minCorner, maxCorner);
      bool x = (id >> 0) & 1, y = (id >> 1) & 1, z = (id >> 2) & 1;
      // default
      Point3D newMinCorner = minCorner;
      Point3D newMaxCorner = maxCorner;
      (x ? newMinCorner.x : newMaxCorner.x) = center.x;
      (y ? newMinCorner.y : newMaxCorner.y) = center.y;
      (z ? newMinCorner.z : newMaxCorner.z) = center.z;
      return {newMinCorner, newMaxCorner};
    }

    void divide() {
      auto center = CENTER(minCorner, maxCorner);
      for (int i = 0; i < CHILD_NUM; ++i) {
        Point3D newMinCorner, newMaxCorner;
        std::tie(newMinCorner, newMaxCorner) = getNewRect(i);
        children[i] =
            std::make_shared<OcNode>(newMinCorner, newMaxCorner, true, *data);
      }
      isLeaf = false;  // not a leaf anymore
      data.reset();    // clear itself data
    }

    bool merge() {
      // check the consistency
      bool valid = true;
      for (int i = 1; i < CHILD_NUM; ++i) {
        if (!children[i]->isLeaf || !children[i - 1]->isLeaf ||
            *(children[i]->data) != *(children[i - 1]->data)) {
          valid = false;
          break;
        }
      }
      if (!valid) return false;

      // merge
      data = std::move(children[0]->data);
      isLeaf = true;

      // clean
      for (int i = 0; i < CHILD_NUM; ++i) {
        children[i].reset();
      }

      return true;
    }

    bool onlyContain1Point() {
      return (maxCorner.x - minCorner.x) == 1 &&
             (maxCorner.y - minCorner.y) == 1 &&
             (maxCorner.z - minCorner.z) == 1;
    }
  };

  std::shared_ptr<OcNode> root;
  std::size_t node_count = 1;  // root
};

}  // namespace ds

#endif  // SRC_DS_OCTREE_HPP_
