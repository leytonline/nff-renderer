// Metal/MetalLayer.h
#import <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <vector>
#include "Nff.h"

@interface MetalLayer : NSObject {
@private
    id<MTLDevice> _dev;
    id<MTLCommandQueue> _cq;
    id<MTLRenderPipelineState> _pipelineState;

    id <MTLBuffer> _verts;
    id <MTLBuffer> _norms;
    id <MTLBuffer> _idx;
    NSUInteger _idxCount;
    id<MTLBuffer> _mvpBuffer;
}
- (void)Render:(uint32_t*)pixels
           pos:(const float*)pos
           dir:(const float*)quat;
- (int)primeBuffers:(const float*) verts
            len:(NSUInteger)len
            vcount:(uint32_t)vcount;
- (void)setMVP:(const float *)mvpData;

@end