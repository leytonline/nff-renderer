// Metal/MetalLayer.mm
#import "MetalLayer.h"

@implementation MetalLayer

- (void)Render:(uint32_t*)pixels
           pos:(const float*)pos
           dir:(const float*)quat {
    NSLog(@"Hello, Metal World!");
}

@end