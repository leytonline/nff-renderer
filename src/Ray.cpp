#include "Ray.h"

Ray::Ray() {
    _dir << 0,0,0;
    _origin << 0,0,0;
    _depth = 0;
}

Ray::Ray(Eigen::Vector3d d, Eigen::Vector3d o) {
    setDir(d);
    setOrigin(o);
    _depth = 0;
}

const Eigen::Vector3d& Ray::getDir() const {
    return _dir;
}

void Ray::setDir(Eigen::Vector3d v) {
    _dir = v;
}

const Eigen::Vector3d& Ray::getOrigin() const {
    return _origin;
}

void Ray::setOrigin(Eigen::Vector3d v) {
    _origin = v;
}

const int& Ray::getDepth() const {
    return _depth;
}

void Ray::setDepth(int d) {
    _depth = d;
}