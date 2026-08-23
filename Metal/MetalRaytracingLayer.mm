#import "MetalRaytracingLayer.h"

@implementation MetalRaytracingLayer

- (instancetype)init {
    self = [super init];
    if (!self) return nil;

    _device = MTLCreateSystemDefaultDevice();
    if (!_device) {
        NSLog(@"Metal is not supported on this device");
        return nil;
    }

    _commandQueue = [_device newCommandQueue];

    NSError* error = nil;
    NSURL* url = [NSURL fileURLWithPath:@"obj/shader.metallib"];
    id<MTLLibrary> library = [_device newLibraryWithURL:url error:&error];
    if (!library) {
        NSLog(@"Failed to load shader.metallib: %@", error.localizedDescription);
        return nil;
    }

    id<MTLFunction> function = [library newFunctionWithName:@"raytrace_main"];
    if (!function) {
        NSLog(@"Failed to load raytrace_main from shader.metallib");
        [library release];
        return nil;
    }

    _pipelineState = [_device newComputePipelineStateWithFunction:function error:&error];
    [function release];
    [library release];
    if (!_pipelineState) {
        NSLog(@"Failed to create ray-tracing compute pipeline: %@", error.localizedDescription);
        return nil;
    }

    return self;
}

- (void)dealloc {
    [_triangles release];
    [_pipelineState release];
    [_commandQueue release];
    [_device release];
    [super dealloc];
}

- (void)setTriangles:(const void*)triangles
               count:(NSUInteger)count
          structSize:(NSUInteger)structSize {
    [_triangles release];
    _triangles = nil;
    _triangleCount = count;

    // Metal does not create zero-length buffers. Keep one unused element for
    // empty scenes; the shader observes triangleCount == 0.
    NSUInteger length = MAX(count, (NSUInteger)1) * structSize;
    if (count == 0) {
        _triangles = [_device newBufferWithLength:length options:MTLResourceStorageModeShared];
    } else {
        _triangles = [_device newBufferWithBytes:triangles
                                          length:length
                                         options:MTLResourceStorageModeShared];
    }
}

- (void)render:(uint32_t*)pixels
         width:(NSUInteger)width
        height:(NSUInteger)height
        camera:(const void*)camera
    cameraSize:(NSUInteger)cameraSize {
    if (!_pipelineState || !_triangles) return;

    id<MTLBuffer> output = [_device newBufferWithLength:width * height * sizeof(uint32_t)
                                                options:MTLResourceStorageModeShared];
    id<MTLBuffer> cameraBuffer = [_device newBufferWithBytes:camera
                                                      length:cameraSize
                                                     options:MTLResourceStorageModeShared];

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    [encoder setComputePipelineState:_pipelineState];
    [encoder setBuffer:output offset:0 atIndex:0];
    [encoder setBuffer:_triangles offset:0 atIndex:1];
    [encoder setBuffer:cameraBuffer offset:0 atIndex:2];

    MTLSize threadsPerGroup = MTLSizeMake(8, 8, 1);
    MTLSize groupCount = MTLSizeMake((width + 7) / 8, (height + 7) / 8, 1);
    [encoder dispatchThreadgroups:groupCount threadsPerThreadgroup:threadsPerGroup];
    [encoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];

    memcpy(pixels, [output contents], width * height * sizeof(uint32_t));
    [cameraBuffer release];
    [output release];
}

@end
