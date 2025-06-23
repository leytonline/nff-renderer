#include "Renderer.h"
#include <iostream>

#ifndef METALRASTERIZER_H
#define METALRASTERIZER_H

#ifndef __APPLE__
#error "Attempting to compile Metal Rasterizer on non-Apple platform"
#endif

#ifdef __OBJC__
@class MetalLayer;
#else
typedef struct objc_object MetalLayer;
#endif

class MetalRasterizer : public Renderer {
public:
    MetalRasterizer();
    ~MetalRasterizer();

    void Render(uint32_t* pixels, const Eigen::Vector3d& pos, const Eigen::Quaterniond& dir) override;
    void SetNff(Nff* n);

private:
    int loadNff();
    struct Impl;  
    Impl* impl;
};

#endif
