#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
};

vertex float4 vertex_main(VertexIn in [[stage_in]],
                          constant float4x4& mvp [[buffer(1)]]) {
    return mvp * float4(in.position, 1.0);
}

fragment float4 frag_main() {
    return float4(1.0, 0.0, 1.0, 1.0); // Bright magenta
}