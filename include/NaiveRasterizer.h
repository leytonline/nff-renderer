#include "Nff.h"
#include "Geometry.h"
#include "Renderer.h"
#include "Eigen/Dense"
#include <stdio.h>
#include <algorithm>
#include <string>

#ifndef RASTER_H
#define RASTER_H

class NaiveRasterizer : public Renderer {
public:
    NaiveRasterizer();
    void Render(uint32_t*, const Eigen::Vector3d&, const Eigen::Quaterniond&);
    Eigen::Matrix4d calcM(const Eigen::Vector3d&, const Eigen::Quaterniond&);
    void processVertices(std::vector<Triangle>&, const Eigen::Vector3d&, Eigen::Matrix4d m);
    void shadeTriangle(const Eigen::Vector3d&, Triangle*, std::vector<Eigen::Vector3d>&);
    void rasterize(std::vector<Triangle>&);
    void raster(Triangle&);
    void writeImage(uint32_t*);
    void processFragments(const Eigen::Vector3d& pos);
    //void SetAxisDebug(bool);
    void SetNff(Nff* n);
public: // separated for vars
    Nff *_nff;
    bool _transparent;
    double _transparency;
    bool _fragmentShading;
    bool _debug;
private:
    std::vector<Fragment> _fragments;
    //void drawDebugAxes(Eigen::Vector3d*, const Eigen::Vector3d&);

};

#endif