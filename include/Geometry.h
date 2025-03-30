#include "Eigen/Dense"

#ifndef GEOMETRY_H
#define GEOMETRY_H

#define EV3d Eigen::Vector3d

class Fill {
public: 
  Fill() : _color(Eigen::Vector3d(0,0,0)), _kd(0.), _ks(0.), _shine(0.), _transmittance(0.), _index(0.) {}
  Eigen::Vector3d _color;
  double _kd, _ks, _shine, _transmittance, _index;
};

class Light {
public:
    Light(double, double, double, double, double, double);
    Eigen::Vector3d _coords;
    Eigen::Vector3d _color;
    bool hasColor;
};

class Geometry {
public:
    Geometry() {_patch = false;}
    bool _patch;
    Fill _fill;
};

class Fragment {
public:
    Fragment(Eigen::Vector3d c, double z) : 
        _color(c), _z(z), _attrNormal(Eigen::Vector3d(0,0,0)), _attrFill(Fill()), _isFragShaded(false), _attrPos(Eigen::Vector3d(0,0,0)) {}
    Eigen::Vector3d _color;
    double _z;
    Eigen::Vector3d _attrNormal;
    Fill _attrFill;
    bool _isFragShaded;
    Eigen::Vector3d _attrPos;
};

class Triangle : public Geometry {
public:
    Triangle(std::vector<EV3d>, Fill);
    friend std::ostream& operator<<(std::ostream&, const Triangle&);
    std::vector<Eigen::Vector3d> _vertices;
    std::vector<Eigen::Vector3d> _shades;
    std::vector<double> _divs;
    std::vector<Eigen::Vector3d> _origVerts;
    std::vector<Eigen::Vector3d> _origNorms;
    bool _normalOverride;
    Eigen::Vector3d _normal;
};

class Tripatch : public Triangle {
public:
    Tripatch(std::vector<EV3d>, std::vector<EV3d>, Fill);
    std::vector<Eigen::Vector3d> _norms;
};

class Sphere : public Geometry {
public:
    Sphere(Eigen::Vector3d, double, Fill);
    Eigen::Vector3d _center;
    friend std::ostream& operator<<(std::ostream&, const Sphere&);
    double _rad;
};

#undef EV3d

#endif