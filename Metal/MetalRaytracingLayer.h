#import <Foundation/Foundation.h>
#include <Metal/Metal.h>

@interface MetalRaytracingLayer : NSObject {
@private
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLComputePipelineState> _pipelineState;
    id<MTLBuffer> _triangles;
    NSUInteger _triangleCount;
}

- (instancetype)init;
- (void)setTriangles:(const void*)triangles
               count:(NSUInteger)count
          structSize:(NSUInteger)structSize;
- (void)render:(uint32_t*)pixels
         width:(NSUInteger)width
        height:(NSUInteger)height
        camera:(const void*)camera
    cameraSize:(NSUInteger)cameraSize;

@end
