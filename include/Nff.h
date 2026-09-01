#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <Eigen/Dense>
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
    std::vector<Geometry*> _surfaces;
    std::vector<Light> _lights;
    Eigen::Vector3d GetFrom() const {
        return _from;
    }
    Eigen::Vector3d GetUp() const {
        return _up;
    }
    Eigen::Vector3d GetAt() const {
        return _at;
    }
private:
    int projectionAxis(const std::vector<Eigen::Vector3d>& verts); 
    Eigen::Vector2d project2d(const Eigen::Vector3d& v, int);
    double cross2d(const Eigen::Vector2d& a, const Eigen::Vector2d& b, const Eigen::Vector2d& c);
    bool inTriangle(const Eigen::Vector2d& a, const Eigen::Vector2d& b, const Eigen::Vector2d& c, const Eigen::Vector2d& p);
    std::vector<std::vector<size_t>> clip(const std::vector<Eigen::Vector3d>& verts);

};

#endif
