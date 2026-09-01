#include <Eigen/Dense>
#include <limits>
#include <ostream>
#include <utility>
#include <vector>
#include "Ray.h"

#ifndef GEOMETRY_H
#define GEOMETRY_H

#define EV3d Eigen::Vector3d

class Fill {
public: 
  Fill() : _color(Eigen::Vector3d(0,0,0)), _kd(0.), _ks(0.), _shine(0.), _transmittance(0.), _index(0.) {}
  Eigen::Vector3d _color;
  double _kd, _ks, _shine, _transmittance, _index;
};

class HitRecord {
public:
    HitRecord() {
        _t = std::numeric_limits<double>::infinity();
        _alpha = 0.;
        _beta = 0.;
        _gamma = 0.;
        _p << 0,0,0;
        _n << 0,0,0;
        _v << 0,0,0;
        _fill = Fill();
        _rayDepth = 0;
        _set = true;
    }
    HitRecord(bool set) : HitRecord() {
        _set = set;
    };
    double _t, _alpha, _beta, _gamma;
    Eigen::Vector3d _p, _n, _v;
    Fill _fill;
    int _rayDepth;
    bool _set; // for checking if set instead of hr != null; 8/29/26 - what?
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
    Geometry() : _vertices(), _patch(false), _fill(Fill()) {};
    Geometry(std::vector<Eigen::Vector3d>, Fill);
    virtual ~Geometry() = default;
    virtual bool intersect(Ray&, double, double, HitRecord&) const = 0;
    virtual std::pair<Eigen::Vector3d, Eigen::Vector3d> getMinMax() const;
    virtual Eigen::Vector3d centroid() const;
    std::vector<Eigen::Vector3d> _vertices;
    bool _patch;
    Fill _fill;
};

class Fragment {
public:
    Fragment() : 
        _color(Eigen::Vector3d::Zero()), _z(std::numeric_limits<double>::infinity()),
        _attrNormal(Eigen::Vector3d(0,0,0)), _attrFill(Fill()), 
        _isFragShaded(false), _attrPos(Eigen::Vector3d(0,0,0)), 
        _lastOpaqueDepth(std::numeric_limits<double>::infinity()), _set(false) {};
    Fragment(Eigen::Vector3d c, double z) : Fragment() {
        _color = c;
        _z = z;
    };
    Eigen::Vector3d _color;
    double _z;
    Eigen::Vector3d _attrNormal;
    Fill _attrFill;
    bool _isFragShaded;
    Eigen::Vector3d _attrPos;
    double _lastOpaqueDepth;
    bool _set;
    const double& getLastOpaqueDepth() const;
    const bool& isSet() const;
};

class Triangle : public Geometry {
public:
    Triangle();
    Triangle(std::vector<EV3d>, Fill);
    void addVertex(Eigen::Vector3d);
    bool intersect(Ray&, double, double, HitRecord&) const;
    friend std::ostream& operator<<(std::ostream&, const Triangle&);
    std::vector<Eigen::Vector3d> _shades;
    std::vector<double> _divs;
    std::vector<Eigen::Vector3d> _origVerts;
    std::vector<Eigen::Vector3d> _origNorms;
    bool _normalOverride;
    Eigen::Vector3d _normal;
};

class Tripatch : public Triangle {
public:
    Tripatch();
    Tripatch(std::vector<EV3d>);
    Tripatch(std::vector<EV3d>, std::vector<EV3d>, Fill);
    void addNorm(Eigen::Vector3d);
    bool intersect(Ray&, double, double, HitRecord&) const;
    bool intersectSmooth(Ray&, double, double, HitRecord&) const;
    std::vector<Eigen::Vector3d> _norms;
};

class Sphere : public Geometry {
public:
    Sphere(Eigen::Vector3d, double, Fill);
    friend std::ostream& operator<<(std::ostream&, const Sphere&);
    bool intersect(Ray&, double, double, HitRecord&) const;
    std::pair<Eigen::Vector3d, Eigen::Vector3d> getMinMax() const;
    const Eigen::Vector3d& getColor() const;
    Eigen::Vector3d centroid() const;
    Eigen::Vector3d _center;
    double _rad;
};

class Polygon : public Geometry {
public:
    Polygon(std::vector<Eigen::Vector3d>);
    bool intersect(Ray&, double, double, HitRecord&) const;
protected:
};

class Polypatch : public Polygon {
public:
    Polypatch(std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>);
private:
    std::vector<Eigen::Vector3d> _norms;
};

#undef EV3d

#endif
