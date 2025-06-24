// Metal/MetalLayer.mm
#import "MetalLayer.h"

@implementation MetalLayer

- (instancetype)init {
    self = [super init];
    if (self) {
        // 1. Create device
        _dev = MTLCreateSystemDefaultDevice();
        if (!_dev) {
            NSLog(@"Metal is not supported on this device");
            return nil;
        }

        // 2. Create command queue
        _cq = [_dev newCommandQueue];

        NSURL* url = [NSURL fileURLWithPath:@"obj/shader.metallib"];
        NSError* err = nil;
        id<MTLLibrary> lib = [_dev newLibraryWithURL:url error:&err];
        if (!lib) {
            NSLog(@"Failed to load shader.metallib: %@", err.localizedDescription);
            return nil;
        }

        id<MTLFunction> vertexFunc = [lib newFunctionWithName:@"vertex_main"];
        id<MTLFunction> fragFunc = [lib newFunctionWithName:@"frag_main"];
        if (!vertexFunc || !fragFunc) {
            NSLog(@"Failed to load shader functions");
            return nil;
        }

        // 4. Define vertex layout
        MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
        vertexDesc.attributes[0].format = MTLVertexFormatFloat3;     // position: float3
        vertexDesc.attributes[0].offset = 0;
        vertexDesc.attributes[0].bufferIndex = 0;

        vertexDesc.layouts[0].stride = sizeof(float) * 3;            // 12 bytes per vertex
        vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

        // 5. Create pipeline descriptor
        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexFunction = vertexFunc;
        desc.fragmentFunction = fragFunc;
        desc.vertexDescriptor = vertexDesc;
        desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

        _pipelineState = [_dev newRenderPipelineStateWithDescriptor:desc error:&err];
        if (!_pipelineState) {
            NSLog(@"Failed to create render pipeline state: %@", err.localizedDescription);
            return nil;
        }
    }
    return self;
}

- (void)Render:(uint32_t*)pixels
           pos:(const float*)pos
           dir:(const float*)quat {

    const NSUInteger width = 512;
    const NSUInteger height = 512;

    MTLTextureDescriptor* texDesc = [
        MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
        width:width
        height:height
        mipmapped:NO
    ];

    texDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;

    id<MTLTexture> targetTex = [_dev newTextureWithDescriptor:texDesc];

    MTLRenderPassDescriptor* rpDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    rpDesc.colorAttachments[0].texture = targetTex;
    rpDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    rpDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.2, 0.2, 0.2, 1.0);
    rpDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLCommandBuffer> cmdBuf = [_cq commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [cmdBuf renderCommandEncoderWithDescriptor:rpDesc];

    [encoder setRenderPipelineState:_pipelineState];
    [encoder setVertexBuffer:_verts offset:0 atIndex:0];
    [encoder setVertexBuffer:_mvpBuffer offset:0 atIndex:1]; // <--- MVP bound

    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:_idxCount];
    [encoder endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];

    // Read GPU texture back to CPU (very slow — good for SDL-style display)
    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    uint8_t* temp = new uint8_t[width * height * 4];
    [targetTex getBytes:temp
            bytesPerRow:width * 4
             fromRegion:region
            mipmapLevel:0];

    memcpy(pixels, temp, width * height * 4);
    delete[] temp;
}
- (int)primeBuffers:(const float*) verts
            len:(NSUInteger)len
            vcount:(uint32_t)vcount {

    _verts = [_dev newBufferWithBytes:verts length:len options:MTLResourceStorageModeManaged];
    _idxCount = vcount;

    return 0;
}

- (void)setMVP:(const float *)mvpData {
    if (!_mvpBuffer) {
        _mvpBuffer = [_dev newBufferWithLength:sizeof(float) * 16 options:MTLResourceStorageModeShared];
    }
    memcpy([_mvpBuffer contents], mvpData, sizeof(float) * 16);
}


@end