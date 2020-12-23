//
//  ViewController.m
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#import <QuartzCore/CAMetalLayer.h>

#import "ViewController.h"
#import "MyMetalView.h"
#import "MyRenderer.h"

#include "application.hpp"

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

- (void)didReceiveEvent:(NSEvent *)event
{
    switch (event.type) {
    case NSEventTypeScrollWheel:
        gl::application::get().scrolled([event scrollingDeltaX], [event scrollingDeltaY]);
    default:
        ;
    }
    /*
     application::get().key_down([event.charactersIgnoringModifiers characterAtIndex:0]);
     application::get().key_up([event.charactersIgnoringModifiers characterAtIndex:0]);
     NSLog(@"%lu", (unsigned long)event.modifierFlags);
     NSLog(@"mouseEntered");
     NSLog(@"mouseExited");
     NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
     application::get().mouse_moved(p.x, p.y);
     application::get().mouse_up([event buttonNumber]);
     application::get().mouse_down([event buttonNumber]);
     NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
     application::get().mouse_moved(p.x, p.y);
     application::get().mouse_up([event buttonNumber]);
     application::get().mouse_down([event buttonNumber]);
     NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
     application::get().mouse_moved(p.x, p.y);
     application::get().mouse_up([event buttonNumber]);
     application::get().mouse_down([event buttonNumber]);
     NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
     application::get().mouse_moved(p.x, p.y);
     application::get().scrolled([event scrollingDeltaX], [event scrollingDeltaY]);

     */
}

@end
