#include "CudaRaytracer.h"

// templates n' prototypes
template<>
CudaTypes::Fill CudaRaytracer::fromNative(Fill f);
template<>
CudaTypes::Light CudaRaytracer::fromNative(Light l);
template<>
float3 CudaRaytracer::fromNative(Eigen::Vector3d v);
template<>
CudaTypes::Triangle CudaRaytracer::fromNative(Tripatch t);
template<>
CudaTypes::Sphere CudaRaytracer::fromNative(Sphere s);

__device__ 
static float3 shade(CudaTypes::SceneHandle*, CudaTypes::HitRecord hr);

__device__
static float3 castRay(CudaTypes::SceneHandle*, CudaTypes::Ray r, float t0, float t1);

// DEVICES 
__device__
static float3 cwiseInverse(float3 v) {
    return float3{
        1.0f / v.x,
        1.0f / v.y,
        1.0f / v.z,
    };
}

__device__ 
static float3 sub(float3 a, float3 b) {
    return float3{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

__device__ 
static float3 add(float3 a, float3 b) {
    return float3{
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

__device__
static float3 mul(float3 a, float f) {
    return float3{
        a.x * f,
        a.y * f,
        a.z * f
    };
}

__device__
static float3 div(float3 a, float f) {
    return float3{
        a.x / f,
        a.y / f,
        a.z / f
    };
}

__device__
static float3 centroid(float3 a, float3 b, float3 c) {
    return div(add(add(a, b), c), 3.0f);
}

__device__
static float3 cross(float3 a, float3 b) {
    return float3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

__device__ 
float dot(float3 a, float3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

__device__
static float lengthSquared(float3 v) {
    return dot(v, v);
}

__device__ 
float length(float3 a) {
    return sqrtf(lengthSquared(a));
}


// later test out leaving length squared, inverting, then multiplying; rsqrtf
__device__
static float3 normalize(float3 v) {
    float len = length(v);
    if (len == 0.f) return float3{0.f, 0.f, 0.f};
    return div(v, len);
}

__device__ 
static bool intersectTriangle(CudaTypes::Triangle t, CudaTypes::Ray r, float t0, float t1, CudaTypes::HitRecord* hr) {
    float eps = 1e-6;

    float3 first = sub(t._v1, t._v0);
    float3 second = sub(t._v2, t._v0);

    float3 normal = cross(first, second);

    if (dot(normal, r._dir) > 0) return false;

    float3 rCrossSec = cross(r._dir, second);
    double det = dot(first, rCrossSec);

    if (fabsf(det) < eps) return false; // ray parallel-"ish" to triangle

    float invDet = 1.0 / det;
    float3 s = sub(r._origin, t._v0);
    float u = dot(s, rCrossSec) * invDet;

    if (u < -eps || u - 1 > eps) return false;

    float3 sCrossFir = cross(s, first);
    float v = invDet * dot(r._dir, sCrossFir);

    if (v < -eps || u + v - 1 > eps) return false;

    float calculated = invDet * dot(second, sCrossFir);

    if (!(calculated > t0 && calculated < t1)) return false;

    hr->_t = calculated;
    hr->_p = add(r._origin, mul(r._dir, calculated));
    hr->_n = normalize(normal);
    hr->_alpha = 1 - u - v;
    hr->_beta = u;
    hr->_gamma = v;
    hr->_fill = t._fill;

    return true;
}

__device__
static bool intersectSphere(CudaTypes::Sphere s, CudaTypes::Ray r, float t0, float t1, CudaTypes::HitRecord* hr) {

    float3 eSubC = sub(r._origin, s._center);

    float a = dot(r._dir, r._dir);
    float b = 2.0f * dot(eSubC, r._dir);
    float c = dot(eSubC, eSubC) - s._radius * s._radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0) return false;

    float quadAdd = (-b + sqrtf(discriminant)) / (2.0f * a);
    float quadSub = (-b - sqrtf(discriminant)) / (2.0f * a);

    float nearT = quadAdd < quadSub ? quadAdd : quadSub;
    float farT = nearT == quadAdd ? quadSub : quadAdd;
    float hit = 0.0f;

    if (nearT > t0 and nearT < t1) hit = nearT;
    else if (farT > t0 && farT < t1) hit = farT;
    else return false;

    hr->_t = hit;
    hr->_p = add(r._origin, mul(r._dir, hit));
    hr->_n = normalize(sub(hr->_p, s._center));
    hr->_fill = s._fill;
    return true;
}

__device__
static float3 castRay(CudaTypes::SceneHandle* sh, CudaTypes::Ray r, float t0, float t1) {
    constexpr uint8_t MAX_BOUNCES = 5;

    CudaTypes::HitRecord hr;
    if (r._depth > MAX_BOUNCES) return sh->_bg;

    bool hit = false;

    for (uint32_t i = 0; i < sh->_numTriangles; i++)
    {
        if (intersectTriangle(sh->_triangles[i], r, t0, t1, &hr))
        {
            hit = true;
            t1 = hr._t;
        }
    } 

    for (uint32_t i = 0; i < sh->_numSpheres; i++)
    {
        if (intersectSphere(sh->_spheres[i], r, t0, t1, &hr))
        {
            hit = true;
            t1 = hr._t;
        }
    } 

    if (hit)
    {
        hr._depth = r._depth;
        hr._v = normalize(sub(r._origin, hr._p));
        return shade(sh, hr);
    }

    return sh->_bg;
}

__device__ 
static float3 shade(CudaTypes::SceneHandle* sh, CudaTypes::HitRecord hr) {

    float3 ret{0.0f, 0.0f, 0.0f};

    float intensity = 1.0f / sqrtf(sh->_numLights);

    CudaTypes::Fill& f = hr._fill;

    for (uint32_t i = 0; i < sh->_numLights; i++)
    {
        CudaTypes::Light& l = sh->_lights[i];

        float3 ld = normalize(sub(l._pos, hr._p));

        CudaTypes::Ray lr {ld, normalize(cwiseInverse(ld)), hr._p, 0};

        bool shadow = false;
        CudaTypes::HitRecord shr;

        for (uint32_t i = 0; i < sh->_numTriangles; i++)
        {
            if (shadow) break;
            shadow = shadow || intersectTriangle(sh->_triangles[i], lr, 1e-6, length(sub(l._pos, hr._p)), &shr);
        } 

        for (uint32_t i = 0; i < sh->_numSpheres; i++)
        {
            if (shadow) break;
            shadow = shadow || intersectSphere(sh->_spheres[i], lr, 1e-6, length(sub(l._pos, hr._p)), &shr);
        } 

        if (!shadow)
        {
            float3 half = normalize(add(ld, hr._v));
            float diffuse = fmaxf(0.0f, dot(hr._n, ld));
            float specular = powf(fmaxf(0.0f, dot(hr._n, half)), f._shine);
            // ret += (f._color * diffuse * f._kd + f._ks*Eigen::Vector3d(specular,specular,specular)) * intensity;
            ret = add(
                ret,
                mul(
                    add(
                        mul(
                            float3{
                                f._r,
                                f._g,
                                f._b
                            },
                            diffuse * f._kd
                        ),
                        mul(
                            float3{
                                specular,
                                specular,
                                specular
                            },
                            f._ks
                        )
                    ),
                    intensity
                )
            );
        }
    }
    
    // ignore reflections + refraction for now
    return ret;
}

__global__ void renderKernel(CudaTypes::SceneHandle* sh, CudaTypes::Camera c) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    if (i >= sh->_width || j >= sh->_height) return;

    float3 color {0.0f, 0.0f, 0.0f};
    
    float a = c._l + i * c._increment;
    float b = c._t - j * c._increment;

    float3 pt = add(add(mul(c._right, a), mul(c._up, b)), add(mul(c._forward, c._distance), c._pos));

    float3 rd = normalize(sub(pt, c._pos));
    float3 id = cwiseInverse(rd);

    CudaTypes::Ray ray{rd, id, c._pos, 0};

    color = castRay(sh, ray, 1e-6, std::numeric_limits<float>::infinity());

    uint8_t red, green, blue;

    float3 normalized {
        fminf(1.0f, fmaxf(0.0f, color.x)) * 255.0f,
        fminf(1.0f, fmaxf(0.0f, color.y)) * 255.0f,
        fminf(1.0f, fmaxf(0.0f, color.z)) * 255.0f
    };

    red = static_cast<uint8_t>(normalized.x);
    green = static_cast<uint8_t>(normalized.y);
    blue = static_cast<uint8_t>(normalized.z);

    sh->_pixels[j * sh->_width + i] = 0xff000000 | (red << 16) | (green << 8) | blue;
}

CudaRaytracer::CudaRaytracer() {
    _samples = 0;
    _jitter = false;
    _phong = false;
    _dof = false;
    _apSize = 0.;
    _gpuScene = nullptr;
    _gpuPixels = nullptr;
}

void CudaRaytracer::SetNff(Nff* nff) {
    _nff = nff;
    _bvh = BVH(nff->_surfaces);

    std::vector<CudaTypes::Sphere> spheres;
    std::vector<CudaTypes::Triangle> triangles;
    std::vector<CudaTypes::Light> lights;


    // construct this later, so when iterating we can assign by it's appearance in BVH so that indices line up
    for (const auto& s : nff->_analyticSpheres)
    {
        spheres.push_back(fromNative<CudaTypes::Sphere>(s));
    }

    for (const auto& t : nff->_tripatches)
    {
        triangles.push_back(fromNative<CudaTypes::Triangle>(t));
    }

    for (const auto& l : nff->_lights)
    {
        lights.push_back(fromNative<CudaTypes::Light>(l));
    }

    CudaTypes::SceneHandle sh {
        triangles.data(),
        static_cast<uint32_t>(triangles.size()), // size_t to uint32_t conversion
        spheres.data(),
        static_cast<uint32_t>(spheres.size()),
        lights.data(),
        static_cast<uint32_t>(lights.size()),
        fromNative<float3>(nff->_bg),
        static_cast<uint32_t>(nff->_res.second),
        static_cast<uint32_t>(nff->_res.first),
        nullptr /* ignore pixels for now*/,
        nullptr /* ignore bvh for now*/,
        0
    };

    _gpuScene = copyToDevice(&sh);
    if (_gpuScene == nullptr)
    {
        std::cerr << "failed to upload scene" << std::endl;
        std::abort();
    }
}

CudaTypes::SceneHandle* CudaRaytracer::copyToDevice(CudaTypes::SceneHandle* sh) {

    CudaTypes::SceneHandle* gpuScene = nullptr;
    CudaTypes::SceneHandle primer{};
    primer._numTriangles = sh->_numTriangles;
    primer._numSpheres = sh->_numSpheres;
    primer._numLights = sh->_numLights;
    primer._bg = sh->_bg;
    cudaError_t cudaErr;

    // cudaMalloc will return a device's pointer to the host
    cudaErr = cudaMalloc(&gpuScene, sizeof(CudaTypes::SceneHandle));
    if (cudaErr != cudaError::cudaSuccess) return nullptr;

    if (sh->_numTriangles > 0)
    {
        size_t sz = sizeof(CudaTypes::Triangle) * sh->_numTriangles;
        cudaErr = cudaMalloc(&primer._triangles, sz);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;

        // cudaMemcpy Host->Device puts the memory at the pointer (primer._triangles is a device pointer on host)
        cudaErr = cudaMemcpy(primer._triangles, sh->_triangles, sz, cudaMemcpyKind::cudaMemcpyHostToDevice);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;
    }
    
    if (sh->_numSpheres > 0)
    {
        size_t sz = sizeof(CudaTypes::Sphere) * sh->_numSpheres;
        cudaErr = cudaMalloc(&primer._spheres, sz);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;

        cudaErr = cudaMemcpy(primer._spheres, sh->_spheres, sz, cudaMemcpyKind::cudaMemcpyHostToDevice);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;
    }

    if (sh->_numLights > 0)
    {
        size_t sz = sizeof(CudaTypes::Light) * sh->_numLights;
        cudaErr = cudaMalloc(&primer._lights, sz);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;

        cudaErr = cudaMemcpy(primer._lights, sh->_lights, sz, cudaMemcpyKind::cudaMemcpyHostToDevice);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;
    }


    primer._height = sh->_height;
    primer._width = sh->_width;

    if (primer._height * primer._width > 0)
    {
        size_t sz = primer._height * primer._width * sizeof(uint32_t);
        cudaErr = cudaMalloc(&primer._pixels, sz);
        if (cudaErr != cudaError::cudaSuccess) return nullptr;
        cudaMemset(primer._pixels, 0, sz);
        _gpuPixels = primer._pixels;
    }

    // bvh setup later ignore for now
    primer._bvh = nullptr;
    primer._bvhSize = 0;

    cudaErr = cudaMemcpy(gpuScene, &primer, sizeof(CudaTypes::SceneHandle), cudaMemcpyKind::cudaMemcpyHostToDevice);
    if (cudaErr != cudaError::cudaSuccess) return nullptr;

    return gpuScene;
}

template<>
CudaTypes::Fill CudaRaytracer::fromNative(Fill f) {
    CudaTypes::Fill ret;
    ret._r = f._color.x();
    ret._g = f._color.y();
    ret._b = f._color.z();
    ret._index = f._index;
    ret._kd = f._kd;
    ret._ks = f._ks;
    ret._shine = f._shine;
    ret._transmittance = f._transmittance;
    return ret;
}

template<>
CudaTypes::Light CudaRaytracer::fromNative(Light l) {
    CudaTypes::Light ret;
    ret._r = l._color.x();
    ret._g = l._color.y();
    ret._b = l._color.z();
    ret._pos.x = l._coords.x();
    ret._pos.y = l._coords.y();
    ret._pos.z = l._coords.z();
    return ret;
}


template<>
float3 CudaRaytracer::fromNative(Eigen::Vector3d v) {
    return float3 {
        static_cast<float>(v.x()),
        static_cast<float>(v.y()),
        static_cast<float>(v.z())
    };
}

template<>
CudaTypes::Triangle CudaRaytracer::fromNative(Tripatch t) {
    CudaTypes::Triangle ret;
    ret._v0 = fromNative<float3>(t._vertices[0]);
    ret._v1 = fromNative<float3>(t._vertices[1]);
    ret._v2 = fromNative<float3>(t._vertices[2]);
    ret._n0 = fromNative<float3>(t._norms[0]);
    ret._n1 = fromNative<float3>(t._norms[1]);
    ret._n2 = fromNative<float3>(t._norms[2]);
    ret._fill = fromNative<CudaTypes::Fill>(t._fill);
    return ret;
}

template<>
CudaTypes::Sphere CudaRaytracer::fromNative(Sphere s) {
    CudaTypes::Sphere ret;
    ret._center = fromNative<float3>(s._center);
    ret._radius = static_cast<float>(s._rad);
    ret._fill = fromNative<CudaTypes::Fill>(s._fill);
    return ret;
}

void CudaRaytracer::Render(uint32_t* out, const Eigen::Vector3d& pos, const Eigen::Quaterniond& orientation) {

    Eigen::Vector3d forward = (orientation * -Eigen::Vector3d::UnitZ()).normalized();
    Eigen::Vector3d right = (orientation * Eigen::Vector3d::UnitX()).normalized();
    Eigen::Vector3d up = (orientation * Eigen::Vector3d::UnitY()).normalized();


    float height = tan((M_PI / 180.0f) * (_nff->_angle / 2.0f)) * (_nff->_from - _nff->_at).norm();
    float increment = (2.0f * height) / _nff->_res.first;

    CudaTypes::Camera camera{
        float3 { // pos
            static_cast<float>(pos.x()),
            static_cast<float>(pos.y()),
            static_cast<float>(pos.z()),
        },
        float3 { // forward
            static_cast<float>(forward.x()),
            static_cast<float>(forward.y()),
            static_cast<float>(forward.z()),
        },
        float3 { // up
            static_cast<float>(up.x()),
            static_cast<float>(up.y()),
            static_cast<float>(up.z()),
        },
        float3 { // right
            static_cast<float>(right.x()),
            static_cast<float>(right.y()),
            static_cast<float>(right.z()),
        },
        height, 
        increment, // increment per pixel
        -height + 0.5f * increment, // _l
        height * (static_cast<float>(_nff->_res.second) / static_cast<float>(_nff->_res.first)) - 0.5f * increment, // t
        static_cast<float>((_nff->_from - _nff->_at).norm()) // ideal distance
    };


    dim3 block(8, 8);
    dim3 grid(
        (_nff->_res.first + block.x - 1) / block.x,
        (_nff->_res.second + block.y - 1) / block.y
    );

    renderKernel<<<grid, block>>>(_gpuScene, camera);

    cudaError_t cudaErr = cudaGetLastError();
    if (cudaErr != cudaSuccess)
    {
        std::cerr << "launch fail" << std::endl;
        std::abort();
    }

    cudaErr = cudaDeviceSynchronize();
    if (cudaErr != cudaSuccess)
    {
        std::cerr << "runtime fail " << cudaGetErrorString(cudaErr) << std::endl;
        std::abort();
    }

    cudaErr = cudaMemcpy(out, _gpuPixels, _nff->_res.first * _nff->_res.second * sizeof(uint32_t), cudaMemcpyKind::cudaMemcpyDeviceToHost);
    if (cudaErr != cudaSuccess)
    {
        std::cerr << "copy back fail" << std::endl;
        std::abort();
    }
}