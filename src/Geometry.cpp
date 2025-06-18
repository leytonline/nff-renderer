#include "Geometry.h"
#include <iostream>

#define EV3d Eigen::Vector3d
Triangle::Triangle(std::vector<EV3d> v, Fill f) : Geometry() {
    _fill = f;
    _vertices = v;
    _normalOverride = false;
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

Tripatch::Tripatch(std::vector<EV3d> v, std::vector<EV3d> n, Fill f) : Triangle(v, f) {
  _patch = true;
  _norms = n;
} 
#undef EV3d

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