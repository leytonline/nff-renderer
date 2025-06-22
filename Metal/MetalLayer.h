// Metal/MetalLayer.h
#import <Foundation/Foundation.h>

@interface MetalLayer : NSObject
- (void)Render:(uint32_t*)pixels
           pos:(const float*)pos
           dir:(const float*)quat;
@end