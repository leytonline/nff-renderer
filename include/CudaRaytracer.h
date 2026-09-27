#pragma once

#include <Eigen/Dense>
#include <cuda_runtime.h>

#include "bvh.h"
#include "Geometry.h"
#include "Renderer.h"
#include "Nff.h"

// Types to be used by Cuda
// divergence reasoning: CUDA has incredible FP32 speed but FP64 is a crazy slowdown in testing, avoid STL containers
// also avoid unecessary dereferencing and runtime polymorphism
// prefer trivially copyable values as well, nothing beyond structs
// and true isolation from CPU native types
namespace CudaTypes {
    struct Fill {
        float _r, _g, _b, _kd, _ks, _shine, _transmittance, _index;
    };

    struct HitRecord {
        float _t;
        // point, normal, direction viewed from
        float3 _p, _n, _v;
        float _alpha, _beta, _gamma;
        CudaTypes::Fill _fill;
        uint8_t _depth;
    };

    struct Light {
        float _r, _g, _b;
        float3 _pos;
    };

    struct Triangle {
        float3 _v0, _v1, _v2, _n0, _n1, _n2;
        CudaTypes::Fill _fill;
    };

    struct Sphere {
        float3 _center;
        float _radius;
        CudaTypes::Fill _fill;
    };

    struct AABB {
        float3 _min;
        float3 _max;
    };

    struct Ray {
        float3 _dir;
        float3 _invDir;
        float3 _origin;
        uint8_t _depth;
    };

    struct Camera {
        float3 _pos, _forward, _up, _right;
        float _h, _increment, _l, _t, _distance;
    };

    struct BVHIndex {
        char _type;
        uint32_t _idx;
    };

    struct BVHNode {
        CudaTypes::AABB _aabb;
        uint32_t _left, _right; // can't handle billions of primitives
        uint32_t _start, _count;
    };

    struct SceneHandle {
        CudaTypes::Triangle* _triangles;
        uint32_t _numTriangles;
        CudaTypes::Sphere* _spheres; // ignored for now
        uint32_t _numSpheres;
        CudaTypes::Light* _lights;
        uint32_t _numLights;
        float3 _bg; // background color
        uint32_t _height; 
        uint32_t _width;
        uint32_t* _pixels;
        CudaTypes::BVHNode* _bvh;
        uint32_t _bvhSize;
    };
};

class CudaRaytracer : public Renderer {
public:
    CudaRaytracer();
    void SetNff(Nff* nff);
    int loadFromFile(std::string);
    __host__
    void Render(uint32_t* out, const Eigen::Vector3d& pos, const Eigen::Quaterniond& orientation);
private:
    // take a scene handle, copy it all onto GPU, return pointer on GPU if successful, otherwise nullptr indicating failure
    __host__ CudaTypes::SceneHandle* copyToDevice(CudaTypes::SceneHandle* sh);
    template<typename T, typename U>
    T fromNative(U);
    int _samples;
    bool _jitter;
    bool _phong;
    bool _dof;
    double _apSize;
    BVH _bvh;
    uint32_t* _gpuPixels;
    CudaTypes::SceneHandle* _gpuScene;
};
