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
#include "Renderer.h"

#ifndef TRACER_H
#define TRACER_H

const int MAX_BOUNCES = 5;

class NaiveRaytracer : public Renderer {
public:
    NaiveRaytracer();
    void SetNff(Nff* nff);
    int loadFromFile(std::string);
    void Render(uint32_t* out, const Eigen::Vector3d& pos, const Eigen::Quaterniond& orientation);
    Eigen::Vector3d castRay(Ray&, double, double);
    Eigen::Vector3d shade(HitRecord&);
    BVH _bvh;
    
    int _samples;
    bool _jitter;
    bool _phong;
    bool _dof;
    double _apSize;
    bool _useBvh;
};

#endif