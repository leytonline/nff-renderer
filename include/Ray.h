#include <Eigen/Dense>

#ifndef RAY_H
#define RAY_H

class Ray {
public:
    Ray();
    Ray(Eigen::Vector3d, Eigen::Vector3d);
    const Eigen::Vector3d& getDir() const;
    void setDir(Eigen::Vector3d);
    const Eigen::Vector3d& getOrigin() const;
    void setOrigin(Eigen::Vector3d);
    const int& getDepth() const;
    void setDepth(int);
private:
    Eigen::Vector3d _dir;
    Eigen::Vector3d _origin;
    int _depth;
};

#endif