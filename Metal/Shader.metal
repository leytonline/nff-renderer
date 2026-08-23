#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 color;
};

vertex VertexOut vertex_main(VertexIn in [[stage_in]], constant float4x4& mvp [[buffer(1)]]) {
    VertexOut out;
    out.position = mvp * float4(in.position, 1.0);
    out.color = in.color;
    return out;
}

fragment float4 frag_main(VertexOut in [[stage_in]]) {
    return float4(in.color, 1.0);
}

struct RayTriangle {
    float4 v0;
    float4 v1;
    float4 v2;
    float4 color;
};

struct RayCamera {
    float4 position;
    float4 right;
    float4 up;
    float4 forwardAndTanHalfFov;
    float4 background;
    uint4 dimensions;
};

static bool intersectTriangle(float3 origin,
                              float3 direction,
                              const device RayTriangle& triangle,
                              thread float& distance) {
    const float epsilon = 1e-6f;
    float3 edge1 = triangle.v1.xyz - triangle.v0.xyz;
    float3 edge2 = triangle.v2.xyz - triangle.v0.xyz;
    float3 p = cross(direction, edge2);
    float determinant = dot(edge1, p);

    // Two-sided intersection so NFF winding does not affect visibility.
    if (abs(determinant) < epsilon) return false;

    float inverseDeterminant = 1.0f / determinant;
    float3 t = origin - triangle.v0.xyz;
    float u = dot(t, p) * inverseDeterminant;
    if (u < 0.0f || u > 1.0f) return false;

    float3 q = cross(t, edge1);
    float v = dot(direction, q) * inverseDeterminant;
    if (v < 0.0f || u + v > 1.0f) return false;

    distance = dot(edge2, q) * inverseDeterminant;
    return distance > epsilon;
}

static uint packBGRA8(float3 color) {
    uint3 rgb = uint3(clamp(color, 0.0f, 1.0f) * 255.0f + 0.5f);
    return rgb.b | (rgb.g << 8) | (rgb.r << 16) | 0xff000000u;
}

kernel void raytrace_main(device uint* pixels [[buffer(0)]],
                          const device RayTriangle* triangles [[buffer(1)]],
                          constant RayCamera& camera [[buffer(2)]],
                          uint2 pixel [[thread_position_in_grid]]) {
    uint width = camera.dimensions.x;
    uint height = camera.dimensions.y;
    if (pixel.x >= width || pixel.y >= height) return;

    float2 uv = (float2(pixel) + 0.5f) / float2(width, height);
    float aspect = float(width) / float(height);
    float screenX = (2.0f * uv.x - 1.0f) * aspect * camera.forwardAndTanHalfFov.w;
    float screenY = (1.0f - 2.0f * uv.y) * camera.forwardAndTanHalfFov.w;
    float3 direction = normalize(camera.forwardAndTanHalfFov.xyz
                               + screenX * camera.right.xyz
                               + screenY * camera.up.xyz);

    float closest = INFINITY;
    int closestTriangle = -1;
    for (uint i = 0; i < camera.dimensions.z; ++i) {
        float distance;
        if (intersectTriangle(camera.position.xyz, direction, triangles[i], distance)
            && distance < closest) {
            closest = distance;
            closestTriangle = int(i);
        }
    }

    float3 color = camera.background.rgb;
    if (closestTriangle >= 0) {
        const device RayTriangle& triangle = triangles[closestTriangle];
        float3 normal = normalize(cross(triangle.v1.xyz - triangle.v0.xyz,
                                        triangle.v2.xyz - triangle.v0.xyz));
        float3 lightDirection = normalize(float3(0.4f, 0.6f, 1.0f));
        float diffuse = 0.2f + 0.8f * abs(dot(normal, lightDirection));
        color = triangle.color.rgb * diffuse;
    }

    pixels[pixel.y * width + pixel.x] = packBGRA8(color);
}
