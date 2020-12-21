//
//  ViewController.m
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//


#import "ViewController.h"
#import "MyMetalView.h"
#import "MyRenderer.h"

#import <QuartzCore/CAMetalLayer.h>

@implementation ViewController
{
    MyRenderer *_renderer;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    
    MyMetalView *view = (MyMetalView *)self.view;
    
    // Set the device for the layer so the layer can create drawable textures that can be rendered to
    // on this device.
    view.metalLayer.device = device;
    
    // Set this class as the delegate to receive resize and render callbacks.
    view.delegate = self;
    
    view.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    
    _renderer = [[MyRenderer alloc] initWithMetalDevice:device
                                      drawablePixelFormat:view.metalLayer.pixelFormat];

}

- (void)drawableResize:(CGSize)size
{
    [_renderer drawableResize:size];
}

- (void)renderToMetalLayer:(nonnull CAMetalLayer *)layer
{
    [_renderer renderToMetalLayer:layer];
}

@end
