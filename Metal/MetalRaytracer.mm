#import "MetalRaytracer.h"
#import "MetalRaytracingLayer.h"
#include <simd/simd.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

struct RayTriangle {
    simd_float4 v0;
    simd_float4 v1;
    simd_float4 v2;
    simd_float4 color;
};

struct RayCamera {
    simd_float4 position;
    simd_float4 right;
    simd_float4 up;
    simd_float4 forwardAndTanHalfFov;
    simd_float4 background;
    simd_uint4 dimensions;
};

simd_float4 toFloat4(const Eigen::Vector3d& value, float w = 0.0f) {
    return {
        static_cast<float>(value.x()),
        static_cast<float>(value.y()),
        static_cast<float>(value.z()),
        w
    };
}

} // namespace

MetalRaytracer::MetalRaytracer()
    : _metal([[MetalRaytracingLayer alloc] init]),
      _verticalFov(45.0f),
      _background(Eigen::Vector3d::Zero()),
      _triangleCount(0) {}

MetalRaytracer::~MetalRaytracer() {
    [_metal release];
}

void MetalRaytracer::SetNff(Nff* nff) {
    _nff = nff;
    _verticalFov = static_cast<float>(nff->_angle);
    _background = nff->_bg;
    loadNff();
}

void MetalRaytracer::loadNff() {
    std::vector<RayTriangle> triangles;
    triangles.reserve(_nff->_surfaces.size());

    for (const Triangle* triangle : _nff->_surfaces) {
        if (triangle->_vertices.size() < 3) continue;

        RayTriangle gpuTriangle;
        gpuTriangle.v0 = toFloat4(triangle->_vertices[0]);
        gpuTriangle.v1 = toFloat4(triangle->_vertices[1]);
        gpuTriangle.v2 = toFloat4(triangle->_vertices[2]);
        gpuTriangle.color = toFloat4(triangle->_fill._color, 1.0f);
        triangles.push_back(gpuTriangle);
    }

    _triangleCount = triangles.size();

    [_metal setTriangles:triangles.data()
                    count:triangles.size()
               structSize:sizeof(RayTriangle)];
}

void MetalRaytracer::Render(uint32_t* pixels,
                            const Eigen::Vector3d& position,
                            const Eigen::Quaterniond& orientation) {
    constexpr NSUInteger width = 1024;
    constexpr NSUInteger height = 1024;

    const Eigen::Vector3d right = orientation * Eigen::Vector3d::UnitX();
    const Eigen::Vector3d up = orientation * Eigen::Vector3d::UnitY();
    const Eigen::Vector3d forward = orientation * -Eigen::Vector3d::UnitZ();
    const float tanHalfFov = std::tan(_verticalFov * 0.5f * static_cast<float>(M_PI) / 180.0f);

    RayCamera camera;
    camera.position = toFloat4(position, 1.0f);
    camera.right = toFloat4(right);
    camera.up = toFloat4(up);
    camera.forwardAndTanHalfFov = toFloat4(forward, tanHalfFov);
    camera.background = toFloat4(_background, 1.0f);
    camera.dimensions = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        static_cast<uint32_t>(_triangleCount),
        0
    };

    [_metal render:pixels
             width:width
            height:height
            camera:&camera
        cameraSize:sizeof(camera)];
}
