//
//  MyRenderer.h
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@interface MyRenderer : NSObject

- (nonnull instancetype)initWithMetalDevice:(nonnull id<MTLDevice>)device
                        drawablePixelFormat:(MTLPixelFormat)drawabklePixelFormat;

- (void)renderToMetalLayer:(nonnull CAMetalLayer*)metalLayer;

- (void)drawableResize:(CGSize)drawableSize;

@end
