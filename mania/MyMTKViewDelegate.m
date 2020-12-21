//
//  MyMTKViewDelegate.m
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

@import simd;
@import MetalKit;

#import "MyMTKViewDelegate.h"

@implementation MyMTKViewDelegate
{
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView
{
    self = [super init];
    if(self)
    {
        _device = mtkView.device;
        
        // Create the command queue
        _commandQueue = [_device newCommandQueue];
    }
    
    return self;
}

/// Called whenever the view needs to render a frame.
- (void)drawInMTKView:(nonnull MTKView *)view
{
    // The render pass descriptor references the texture into which Metal should draw
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor == nil)
    {
        return;
    }
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    
    // Create a render pass and immediately end encoding, causing the drawable to be cleared
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    
    [commandEncoder endEncoding];
    
    // Get the drawable that will be presented at the end of the frame
    
    id<MTLDrawable> drawable = view.currentDrawable;
    
    // Request that the drawable texture be presented by the windowing system once drawing is done
    [commandBuffer presentDrawable:drawable];
    
    [commandBuffer commit];
}


/// Called whenever view changes orientation or is resized
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
}


@end
