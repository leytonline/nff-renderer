// Metal/MetalRasterizer.mm
#import "MetalRasterizer.h"
#import "MetalLayer.h"
#include <Eigen/Dense>
using namespace Eigen;

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

static Matrix4f makeMVP(const Vector3d& pos, const Quaterniond& q) {
    Matrix4f proj = Matrix4f::Identity();
    float near = 0.1f, far = 100.0f, fov = 45.0f;
    float aspect = 1.0f;  // square screen for now

    float f = 1.0f / tan(fov * 0.5f * M_PI / 180.0f);
    proj(0,0) = f / aspect;
    proj(1,1) = f;
    proj(2,2) = (far + near) / (near - far);
    proj(2,3) = (2.0f * far * near) / (near - far);
    proj(3,2) = -1.0f;
    proj(3,3) = 0.0f;

    // Convert Quaternion to rotation matrix
    Matrix3f rot = q.cast<float>().toRotationMatrix();
    Matrix4f view = Matrix4f::Identity();
    view.block<3,3>(0,0) = rot.transpose();  // inverse rotation
    view.block<3,1>(0,3) = -rot.transpose() * pos.cast<float>(); // inverse translation

    return proj * view;
}

void MetalRasterizer::Render(uint32_t* pixels, const Eigen::Vector3d& pos, const Eigen::Quaterniond& dir) {
    float pos_f[3] = { static_cast<float>(pos.x()), static_cast<float>(pos.y()), static_cast<float>(pos.z()) };
    float quat_f[4] = { static_cast<float>(dir.w()), static_cast<float>(dir.x()), static_cast<float>(dir.y()), static_cast<float>(dir.z()) };

      // Generate MVP
    Eigen::Matrix4f mvp = makeMVP(pos, dir);
    [impl->metal setMVP:mvp.data()];  // upload to Metal


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

    NSLog(@"total of verts: %zu", verts.size());
    NSLog(@"should have: %zu", _nff->_surfaces.size());
    _nff->dumpLong();

    [impl->metal primeBuffers:verts.data()
                     len:verts.size() * sizeof(float)
                  vcount:verts.size() / 3];

    return 0;
}
