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
    void Render(uint32_t*, const Eigen::Vector3d&);
    Eigen::Matrix4d calcM(const Eigen::Vector3d&);
    void processVertices(std::vector<Triangle>&, Eigen::Matrix4d m);
    void shadeTriangle(Triangle*, std::vector<Eigen::Vector3d>&);
    void rasterize(std::vector<Triangle>&, std::vector<Fragment>*);
    void raster(Triangle&, std::vector<Fragment>*);
    void blend(std::vector<Fragment>*, Eigen::Vector3d*);
    void writeImage(Eigen::Vector3d*, uint32_t*);
    void processFragments(std::vector<Fragment>*);
    void SetAxisDebug(bool);
    void SetNff(Nff* n);
public: // separated for vars
    Nff *_nff;
    bool _transparent;
    double _transparency;
    bool _fragmentShading;
    bool _debug;
private:
    void drawDebugAxes(Eigen::Vector3d*, const Eigen::Vector3d&);

};

#endif