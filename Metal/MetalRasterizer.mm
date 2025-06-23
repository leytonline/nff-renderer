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
        [metal release];  
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

void MetalRasterizer::SetNff(Nff* n) {
    _nff = n;
    loadNff();
}

int MetalRasterizer::loadNff() {

    std::vector<float> verts;

    for (std::vector<Triangle*>::const_iterator itr = _nff->_surfaces.begin(); itr != _nff->_surfaces.end(); ++itr) 
    {
        for (unsigned i = 0; i < 3; i++) 
        {
            verts.push_back((float)(*itr)->_vertices[i].x());
            verts.push_back((float)(*itr)->_vertices[i].y());
            verts.push_back((float)(*itr)->_vertices[i].z());
        }
    }

    [impl->metal primeBuffers:verts.data()
                     len:verts.size() * sizeof(float)
                  vcount:verts.size() / 3];

    return 0;
}