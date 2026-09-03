#include "bvh.h"

// AABB
AABB::AABB() {
    _bounds = std::make_pair(Eigen::Vector3d(-1,-1,-1), Eigen::Vector3d(-1,-1,-1));
}

AABB::AABB(Eigen::Vector3d min, Eigen::Vector3d max) {
    _bounds = std::make_pair(min, max);
}

const Eigen::Vector3d& AABB::min() const {
    return _bounds.first;
}

const Eigen::Vector3d& AABB::max() const {
    return _bounds.second;
}

// https://tavianator.com/2011/ray_box.html
bool AABB::intersects(const Ray& r, double t0, double t1) const {
    double tx1, tx2, ty1, ty2, tz1, tz2;

    double tmin, tmax;

    Eigen::Vector3d rayInvDir = r.getDir().cwiseInverse();

    tx1 = (min().x() - r.getOrigin().x()) * rayInvDir.x();
    tx2 = (max().x() - r.getOrigin().x()) * rayInvDir.x();

    ty1 = (min().y() - r.getOrigin().y()) * rayInvDir.y();
    ty2 = (max().y() - r.getOrigin().y()) * rayInvDir.y();

    tz1 = (min().z() - r.getOrigin().z()) * rayInvDir.z();
    tz2 = (max().z() - r.getOrigin().z()) * rayInvDir.z();

    tmin = std::min(tx1, tx2);
    tmin = std::max(tmin, std::min(ty1, ty2));
    tmin = std::max(tmin, std::min(tz1, tz2));

    tmax = std::max(tx1, tx2);
    tmax = std::min(tmax, std::max(ty1, ty2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    tmax = std::min(t1, tmax);
    tmin = std::max(t0, std::max(tmin, 0.0));

    return tmax >= tmin;
}

double AABB::surfaceArea() const {
    Eigen::Vector3d diff = max() - min();

    return 2.0 * (
        diff.z() * diff.x() +
        diff.x() * diff.y() +
        diff.y() * diff.z()
    );
}

// BVHNode
BVHNode::BVHNode() : _count(0) {
    _left = _right = -1;
}

BVHNode::BVHNode(AABB aabb) : BVHNode() {
    _aabb = aabb;
}

bool BVHNode::isLeaf() {
    return _count > 0;
}

void BVHNode::setLeft(size_t left) {
    _left = left;
}

void BVHNode::setRight(size_t right) {
    _right = right;
}

void BVHNode::setIndices(size_t start, size_t count) {
    _startIdx = start;
    _count = count;
}

size_t BVHNode::getStart() const {
    return _startIdx;
}

size_t BVHNode::getCount() const {
    return _count;
}

const AABB& BVHNode::getAABB() const {
    return _aabb;
}

long long BVHNode::getLeft() const {
    return _left;
}

long long BVHNode::getRight() const {
    return _right;
}

BVH::BVH() : _geos(nullptr) {

}

// BVH
BVH::BVH(std::vector<Geometry*>& geo) {
    _indices.resize(geo.size());
    std::iota(_indices.begin(), _indices.end(), 0);
    construct(geo, _indices);
    _geos = &geo;
}

bool BVH::intersect(Ray& r, double t0, double t1, HitRecord& hr) {
    if (!_nodes.size())
    {
        std::cerr << __PRETTY_FUNCTION__ << ": empty BVH" << std::endl; // should handle gracefully, lazy tho
        std::abort();
    }
    
    bool ret = false;

    std::stack<size_t> st;
    st.push(0);

    while (!st.empty())
    {
        size_t idx = st.top();
        st.pop();

        BVHNode& curr = _nodes[idx];

        if (!curr.getAABB().intersects(r, t0, t1))
        {
            continue;
        }

        if (curr.isLeaf())
        {
            // intersect the geometry
            int offset = curr.getStart();
            for (int i = 0; i < (int) curr.getCount(); i++)
            {
                int idx = _indices[offset + i];
                if ( (*_geos)[idx]->intersect(r, t0, t1, hr) )
                {
                    ret = true;
                    t1 = hr._t;
                }
            }
        }
        else
        {
            st.push(curr.getLeft());
            st.push(curr.getRight());
        }
    }
    

    return ret;
}

bool BVH::intersectAny(Ray& r, double t0, double t1) {
    if (!_nodes.size())
    {
        std::cerr << __PRETTY_FUNCTION__ << ": empty BVH" << std::endl; // should handle gracefully, lazy tho
        std::abort();
    }
    
    HitRecord hr; // doesn't get used, but need for param

    std::stack<size_t> st;
    st.push(0);

    while (!st.empty())
    {
        size_t idx = st.top();
        st.pop();

        BVHNode& curr = _nodes[idx];

        if (!curr.getAABB().intersects(r, t0, t1))
        {
            continue;
        }

        if (curr.isLeaf())
        {
            // intersect the geometry
            int offset = curr.getStart();
            for (int i = 0; i < (int) curr.getCount(); i++)
            {
                int idx = _indices[offset + i];
                if ( (*_geos)[idx]->intersect(r, t0, t1, hr) ) return true;
            }
        }
        else
        {
            st.push(curr.getLeft());
            st.push(curr.getRight());
        }
    }
    

    return false;
}

void BVH::construct(std::vector<Geometry*>& geo, std::span<size_t> indices) {

    // Create an BVHNode with an AABB around all of the triangles in geo
    auto [aabbMin, aabbMax] = geo.at(indices.front())->getMinMax();

    for (size_t n : indices)
    {
        auto [min, max] = geo[n]->getMinMax();
        aabbMin = aabbMin.cwiseMin(min);
        aabbMax = aabbMax.cwiseMax(max);
    }

    AABB box(aabbMin, aabbMax);
    _nodes.push_back(BVHNode(box));

    // type narrowed, going to ignore for now
    int idx = _nodes.size() - 1;

    if (indices.size() <= MAX_PRIMITIVES)
    {
        _nodes.back().setIndices(indices.data() - _indices.data(), indices.size());
        return;
    }

    // Determine the axis to split on (handled internally by median splitter)
    // Determine the median on that axis to separate into two sets
    size_t splitIdx = medianOfSplit(geo, indices);

    std::span<size_t> left = indices.first(splitIdx);
    std::span<size_t> right = indices.subspan(splitIdx);


    // Recursively create children, set left/right indices
    _nodes.back().setLeft(idx + 1);
    construct(geo, left);

    _nodes.at(idx).setRight(_nodes.size());
    construct(geo, right);

    // done

}

// base it on the span indices
AXIS BVH::determineAxisSplit(std::span<Geometry* const> geo, std::span<size_t> indices) {
    if (geo.empty()) 
    {   
        std::cerr << __PRETTY_FUNCTION__ << ": empty triangle span" << std::endl;
        std::abort();
    }

    auto [smallest, largest] = geo[indices.front()]->getMinMax();
    for (size_t i : indices) {
        auto [min, max] = geo[i]->getMinMax();
        smallest = smallest.cwiseMin(min);
        largest = largest.cwiseMax(max);
    }

    Eigen::Vector3d diffs = largest - smallest;

    if (diffs.x() >= diffs.y() && diffs.x() >= diffs.z()) return AXIS::X;
    if (diffs.y() >= diffs.z()) return AXIS::Y;

    return AXIS::Z;
}

size_t BVH::medianOfSplit(const std::vector<Geometry*>& geo, std::span<size_t> indices) {
    AXIS axis = determineAxisSplit(geo, indices);

    auto mid = indices.begin() + indices.size() / 2;
 
    std::nth_element(
        indices.begin(), 
        mid, 
        indices.end(),
        [axis, &geo](size_t i, size_t j) {
            return geo[i]->centroid()[axis] < geo[j]->centroid()[axis];
        }
    );

    return std::distance(indices.begin(), mid);
}