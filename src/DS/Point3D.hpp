// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_DS_POINT3D_HPP_
#define SRC_DS_POINT3D_HPP_

namespace ds {

struct Point3D {
  int x;
  int y;
  int z;

  bool operator<(const Point3D &other) const {
    return (x < other.x) && (y < other.y) && (z < other.z);
  }

  bool operator<=(const Point3D &other) const {
    return (x <= other.x) && (y <= other.y) && (z <= other.z);
  }

  bool operator>(const Point3D &other) const {
    return (x > other.x) && (y > other.y) && (z > other.z);
  }

  bool operator>=(const Point3D &other) const {
    return (x >= other.x) && (y >= other.y) && (z >= other.z);
  }

  bool operator==(const Point3D &other) const {
    return (x == other.x) && (y == other.y) && (z == other.z);
  }

  bool operator!=(const Point3D &other) const {
    return (x != other.x) || (y != other.y) || (z != other.z);
  }
};

}  // namespace ds

#endif  // SRC_DS_POINT3D_HPP_
