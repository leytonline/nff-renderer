#include "Nff.h"
#include "Geometry.h"
#include "Eigen/Dense"
#include <stdio.h>
#include <algorithm>
#include <string>

#ifndef RASTER_H
#define RASTER_H

class Rasterizer {
public:
    Rasterizer(std::string);
    void pipeline(uint32_t*);
    Eigen::Matrix4d calcM();
    void processVertices(std::vector<Triangle>&);
    void shadeTriangle(Triangle*, std::vector<Eigen::Vector3d>&);
    void rasterize(std::vector<Triangle>&, std::vector<Fragment>*);
    void raster(Triangle&, std::vector<Fragment>*);
    void blend(std::vector<Fragment>*, Eigen::Vector3d*);
    void writeImage(Eigen::Vector3d*, uint32_t*);
    void processFragments(std::vector<Fragment>*);
public: // separated for vars
    Nff _nff;
    bool _transparent;
    double _transparency;
    std::string _in;
    bool _fragmentShading;

};

#endif