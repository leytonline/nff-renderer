#include "Geometry.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>

const double& getLastOpaqueDepth();
const bool& isSet();

#define EV3d Eigen::Vector3d
// <GEOMETRY>
Geometry::Geometry(std::vector<Eigen::Vector3d> verts, Fill fill) : 
    _vertices(verts), _patch(false), _fill(fill) {};

Eigen::Vector3d Geometry::centroid() const {
    if (_vertices.size() == 0) return Eigen::Vector3d{0,0,0};

    Eigen::Vector3d center = std::accumulate(
      _vertices.begin(), 
      _vertices.end(), 
      Eigen::Vector3d{0,0,0}
    );
    return center / _vertices.size();
}

std::pair<Eigen::Vector3d, Eigen::Vector3d> Geometry::getMinMax() const {
    if (_vertices.size() == 0) return {Eigen::Vector3d{0,0,0}, Eigen::Vector3d{0,0,0}};

    Eigen::Vector3d min = _vertices.front(), max = _vertices.front();

    for (const Eigen::Vector3d& v : _vertices)
    {
        min = min.cwiseMin(v);
        max = max.cwiseMax(v);
    }

    return {min, max};
}

// <TRIANGLE>
Triangle::Triangle() : Geometry() {
    _normalOverride = false;
}

Triangle::Triangle(std::vector<EV3d> v, Fill f) : Geometry(v, f) {
    _normalOverride = false;
}

// add some vertex in v[0], v[1], v[2] order
void Triangle::addVertex(Eigen::Vector3d v) {
    _vertices.push_back(v);
}

// operator for debugging purposes
std::ostream& operator<<(std::ostream& os, const Triangle& t) {
    std::stringstream builder;
    builder << "Triangle: (";

    unsigned i;

    for (i = 0; i < t._vertices.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            builder << t._vertices[i][j];
            if (j < 2) builder << ' ';
        }
        if (i < 2) builder << ", ";
    }
    builder << ')';

    builder << t._fill._color[0] << ' ' << t._fill._color[1] << ' ' << t._fill._color[2];

    os << builder.str();

    return os;
}

// moller trumbore
bool Triangle::intersect(Ray& r, double t0, double t1, HitRecord& hr) const {  
  constexpr double eps = 1e-6;

  Eigen::Vector3d first = _vertices[1] - _vertices[0], second = _vertices[2] - _vertices[0];

  Eigen::Vector3d normal = first.cross(second);
  if (_normalOverride) normal = _normal;
  if (normal.dot(r.getDir()) > 0) return false; //backface cull, "behind" this triangle

  Eigen::Vector3d rCrossSec = r.getDir().cross(second);
  double det = first.dot(rCrossSec);

  if (std::abs(det) < eps) return false; // parallel to triangle case

  double invDet = 1.0 / det;
  Eigen::Vector3d s = r.getOrigin() - _vertices[0];
  float u = invDet * s.dot(rCrossSec);

  if (u < -eps || u - 1 > eps) return false;

  Eigen::Vector3d sCrossFir = s.cross(first);
  double v = invDet * r.getDir().dot(sCrossFir);

  if (v < -eps || u + v - 1 > eps) return false;

  double t = invDet * second.dot(sCrossFir);

  if (t > t0 && t < t1)
  {
     if (hr._set)
        {
          hr._t = t;
          hr._p = r.getOrigin() + t * r.getDir();
          hr._n = normal;
          hr._n.normalize();
          hr._alpha = 1.0 - u - v;
          hr._beta = u;
          hr._gamma = v;
          hr._fill = _fill;
        }
        return true;
  }

  return false;

}

// <TRIPATCH>
Tripatch::Tripatch() : Triangle() {
  _patch = true;
} 

Tripatch::Tripatch(std::vector<EV3d> n) : Triangle() {
    _norms = n;
    _patch = true;
}

Tripatch::Tripatch(std::vector<EV3d> v, std::vector<EV3d> n, Fill f) : Triangle(v, f) {
  _patch = true;
  _norms = n;
} 

void Tripatch::addNorm(Eigen::Vector3d n) {
    _norms.push_back(n);
}

bool Tripatch::intersect(Ray& r, double t0, double t1, HitRecord& hr) const {
    return intersectSmooth(r, t0, t1, hr);
}

// intersect function
bool Tripatch::intersectSmooth(Ray& r, double t0, double t1, HitRecord& hr) const {
    bool retval = Triangle::intersect(r, t0, t1, hr);

    // calculate intersect as normal, then just use this normal on the patch instead
    if (retval)
    {
      hr._n = _norms[0] + hr._beta * (_norms[1] - _norms[0]) + hr._gamma * (_norms[2] - _norms[0]);
      hr._n.normalize();
    }
    return retval;
}
#undef EV3d

// <SPHERE>
Sphere::Sphere(Eigen::Vector3d c, double r, Fill fill) {
    _center = c;
    _rad = r;
    _fill = fill;
}

std::ostream& operator<<(std::ostream& os, const Sphere& s) {
    os << "Sphere: (" << s._center[0] << ", " << s._center[1] << ", " << s._center[2]
    << ") radius: " << s._rad;
    return os;
}

bool Sphere::intersect(Ray& r, double t0, double t1, HitRecord& hr) const {

    // e-c, since its reused a lot
    Eigen::Vector3d eSubC = r.getOrigin() - _center;

    double b = 2 * eSubC.transpose() * r.getDir();
    double a = r.getDir().transpose() * r.getDir();
    double c = eSubC.transpose() * eSubC - _rad * _rad;
    
    double discriminant = b*b - 4 * a * c;

    if (discriminant < 0) return false;

    // quadratic formulas for +/- sqrt(discrim)
    double quadAdd = (-b + sqrt(discriminant)) / (2 * a);
    double quadSub = (-b - sqrt(discriminant)) / (2 * a);

    double nearT = std::min(quadAdd, quadSub);
    double farT = std::max(quadAdd, quadSub);
    double hitT = 0.;

    if (nearT > t0 && nearT < t1)
    {
      hitT = nearT;
    }
    else if (farT > t0 && farT < t1)
    {
      hitT = farT;
    }
    else
    {
      return false;
    }

    if (hr._set)
    {
      hr._t = hitT;
      hr._p = r.getOrigin() + hitT * r.getDir();
      hr._n = (hr._p - _center) / _rad;
      hr._fill = _fill;
    }
    return true;
}

std::pair<Eigen::Vector3d, Eigen::Vector3d> Sphere::getMinMax() const {
  return {
    _center - Eigen::Vector3d(_rad, _rad, _rad),
    _center + Eigen::Vector3d(_rad, _rad, _rad)
  };
}

const Eigen::Vector3d& Sphere::getColor() const {
  return _fill._color;
}

// <GEOMETRY>
Eigen::Vector3d Sphere::centroid() const {
  return _center;
}

Light::Light(double x, double y, double z, double r, double g, double b) {
  _coords[0] = x;
  _coords[1] = y;
  _coords[2] = z;

  if (r == -1) hasColor = false;
  else hasColor = true;

  _color[0] = r;
  _color[1] = g;
  _color[2] = b;
}

const double& Fragment::getLastOpaqueDepth() const {
    return _lastOpaqueDepth;
}

const bool& Fragment::isSet() const {
    return _set;
}
