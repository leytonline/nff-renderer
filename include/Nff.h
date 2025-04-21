#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "Eigen/Dense"
#include "Geometry.h"

#ifndef NFF_H
#define NFF_H

class Nff {
public:
    Nff();
    ~Nff();
    void clear();
    int parse(std::string);
    void dump();
    void dumpLong(bool = false);
    void calcDir();
    Eigen::Vector3d _u;
    Eigen::Vector3d _v;
    Eigen::Vector3d _w;
    Eigen::Vector3d _from;
    Eigen::Vector3d _at;
    Eigen::Vector3d _bg;
    Eigen::Vector3d _up;
    Eigen::Vector3d _maximums;
    double _angle;
    double _hither;
    std::pair<int, int> _res;
    std::vector<Triangle*> _surfaces;
    std::vector<Light> _lights;
    Eigen::Vector3d GetFrom() const {
        return _from;
    }
    Eigen::Vector3d GetUp() const {
        return _up;
    }
    Eigen::Vector3d GetAt() const {
        return _up;
    }
};

#endif