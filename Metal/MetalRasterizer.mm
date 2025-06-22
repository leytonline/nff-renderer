// Metal/MetalRasterizer.mm
#import "MetalRasterizer.h"
#import "MetalLayer.h"
#include <Eigen/Dense>

struct MetalRasterizer::Impl {
    MetalLayer* metal;

    Impl() {
        metal = [[MetalLayer alloc] init];
    }

    ~Impl() {
        [metal release];  // if not using ARC
    }

    void draw(uint32_t* pixels, const float* pos, const float* quat) {
        [metal Render:pixels pos:pos dir:quat];
    }
};

MetalRasterizer::MetalRasterizer() {
    impl = new Impl();
}

MetalRasterizer::~MetalRasterizer() {
    delete impl;
}

void MetalRasterizer::Render(uint32_t* pixels, const Eigen::Vector3d& pos, const Eigen::Quaterniond& dir) {
    float pos_f[3] = { static_cast<float>(pos.x()), static_cast<float>(pos.y()), static_cast<float>(pos.z()) };
    float quat_f[4] = { static_cast<float>(dir.w()), static_cast<float>(dir.x()), static_cast<float>(dir.y()), static_cast<float>(dir.z()) };

    impl->draw(pixels, pos_f, quat_f);
}