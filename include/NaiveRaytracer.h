#include <cmath>
#include <algorithm>
#include <random>
#include <vector>
#include <ctime>
#include "Nff.h"
#include "Geometry.h"
#include "Ray.h"
#include "bvh.h"
#include <Eigen/Dense>
#include <numeric>
#include <omp.h>

#ifndef TRACER_H
#define TRACER_H

const int MAX_BOUNCES = 5;

class Tracer {
public:
    Tracer();
    Tracer(std::string);
    int loadFromFile(std::string);
    int trace(char*);
    Eigen::Vector3d castRay(Ray&, double, double);
    Eigen::Vector3d shade(HitRecord&);
    Nff _image;
    BVH _bvh;
    
    int _samples;
    bool _jitter;
    bool _phong;
    bool _dof;
    double _apSize;
    bool _useBvh;
};

#endif