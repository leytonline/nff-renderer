#include "Renderer.h"
#include <cstddef>

#ifndef METALRAYTRACER_H
#define METALRAYTRACER_H

#ifndef __APPLE__
#error "Attempting to compile Metal ray tracer on non-Apple platform"
#endif

#ifdef __OBJC__
@class MetalRaytracingLayer;
#else
typedef struct objc_object MetalRaytracingLayer;
#endif

class MetalRaytracer : public Renderer {
public:
    MetalRaytracer();
    ~MetalRaytracer() override;
    void Render(uint32_t*, const Eigen::Vector3d&, const Eigen::Quaterniond&) override;
    void SetNff(Nff*);

private:
    void loadNff();

    MetalRaytracingLayer* _metal;
    float _verticalFov;
    Eigen::Vector3d _background;
    std::size_t _triangleCount;
};

#endif
