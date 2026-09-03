#ifndef BVH_H
#define BVH_H

#include <iostream>
#include <vector>
#include <span>
#include <algorithm>
#include <utility>
#include <stack>
#include <numeric>

#include <Eigen/Dense>
#include "Geometry.h"
#include "Ray.h"

constexpr size_t MAX_PRIMITIVES = 4;

enum AXIS : size_t {X, Y, Z};

class AABB {
public:
    AABB();
    AABB(Eigen::Vector3d min, Eigen::Vector3d max);
    const Eigen::Vector3d& min() const;
    const Eigen::Vector3d& max() const;
    double surfaceArea() const;
    bool intersects(const Ray&, double t0, double t1) const;
private:
    // min is first, max is second
    std::pair<Eigen::Vector3d, Eigen::Vector3d> _bounds;
};

class BVHNode {
public:
    BVHNode();
    BVHNode(AABB);
    bool isLeaf();
    void setLeft(size_t);
    void setRight(size_t);
    long long getLeft() const;
    long long getRight() const;
    void setIndices(size_t start, size_t count);
    size_t getStart() const;
    size_t getCount() const;
    const AABB& getAABB() const;
private:
    AABB _aabb;
    long long _left, _right; // for the owning BVH, which indices are it's children
    size_t _startIdx, _count; // node's first primitive's index (in BVH class) and for how many it continues for
};
 
class BVH {
public: 
    BVH();
    BVH(std::vector<Geometry*>& geo);
    bool intersect(Ray&, double, double, HitRecord&);
    bool intersectAny(Ray&, double, double);
private:
    void construct(std::vector<Geometry*>&);
    void construct(std::vector<Geometry*>&, std::span<size_t> indices);
    AXIS determineAxisSplit(std::span<Geometry* const>, std::span<size_t> indices);
    size_t medianOfSplit(const std::vector<Geometry*>& geo, std::span<size_t>);
    std::vector<size_t> _indices; // indices to original triangle in leaf-order
    std::vector<BVHNode> _nodes;
    std::vector<Geometry*>* _geos;
};


#endif // BVH_H