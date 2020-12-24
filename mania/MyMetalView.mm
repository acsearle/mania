//
//  MyMetalView.mm
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#import "MyMetalView.h"

#include "application.hpp"
#include "renderer.hpp"

#include <cinttypes>


using manic::application;

@implementation MyMetalView
{
    CVDisplayLinkRef _displayLink;
}


- (instancetype) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if(self)
    {
        [self initCommon];
    }
    return self;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        [self initCommon];
    }
    return self;
}

- (void)initCommon
{
    self.wantsLayer = YES;
    
    self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
    
    _metalLayer = (CAMetalLayer*) self.layer;
    
    self.layer.delegate = self;
}

- (CALayer *)makeBackingLayer
{
    return [CAMetalLayer layer];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    
    [self setupCVDisplayLinkForScreen:self.window.screen];

    [self resizeDrawable:self.window.screen.backingScaleFactor];
}


- (BOOL)setupCVDisplayLinkForScreen:(NSScreen*)screen
{
    CVReturn cvReturn;
    
    // Create a display link capable of being used with all active displays
    cvReturn = CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    
    if(cvReturn != kCVReturnSuccess)
    {
        return NO;
    }
        
    // Set DispatchRenderLoop as the callback function and
    // supply this view as the argument to the callback.
    cvReturn = CVDisplayLinkSetOutputCallback(_displayLink, &DispatchRenderLoop, (__bridge void*)self);
        
    if(cvReturn != kCVReturnSuccess)
    {
        return NO;
    }
    
    // Associate the display link with the display on which the
    // view resides
    CGDirectDisplayID viewDisplayID =
    (CGDirectDisplayID) [self.window.screen.deviceDescription[@"NSScreenNumber"] unsignedIntegerValue];;
    
    cvReturn = CVDisplayLinkSetCurrentCGDisplay(_displayLink, viewDisplayID);
    
    if(cvReturn != kCVReturnSuccess)
    {
        return NO;
    }
    
    CVDisplayLinkStart(_displayLink);
    
    NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
    
    // Register to be notified when the window closes so that you
    // can stop the display link
    [notificationCenter addObserver:self
                           selector:@selector(windowWillClose:)
                               name:NSWindowWillCloseNotification
                             object:self.window];
    
    return YES;
}

- (void)windowWillClose:(NSNotification*)notification
{
    // Stop the display link when the window is closing since there
    // is no point in drawing something that can't be seen
    if(notification.object == self.window)
    {
        CVDisplayLinkStop(_displayLink);
    }
}

static CVReturn DispatchRenderLoop(CVDisplayLinkRef displayLink,
                                   const CVTimeStamp* now,
                                   const CVTimeStamp* outputTime,
                                   CVOptionFlags flagsIn,
                                   CVOptionFlags* flagsOut,
                                   void* displayLinkContext)
{
    // printf("leading by %" PRIu64 " ticks\n", (outputTime->hostTime - now->hostTime));
    @autoreleasepool
    {
        MyMetalView *customView = (__bridge MyMetalView*)displayLinkContext;
        [customView render];
    }
    return kCVReturnSuccess;
}



- (void)stopRenderLoop
{
    if(_displayLink)
    {
        // Stop the display link BEFORE releasing anything in the view otherwise the display link
        // thread may call into the view and crash when it encounters something that no longer
        // exists
        CVDisplayLinkStop(_displayLink);
        CVDisplayLinkRelease(_displayLink);
    }
}

- (void)dealloc
{
    [self stopRenderLoop];
}



- (void)resizeDrawable:(CGFloat)scaleFactor
{
    CGSize newSize = self.bounds.size;
    newSize.width *= scaleFactor;
    newSize.height *= scaleFactor;
    
    if(newSize.width <= 0 || newSize.width <= 0)
    {
        return;
    }
    
    // All AppKit and UIKit calls which notify of a resize are called on the main thread.  Use
    // a synchronized block to ensure that resize notifications on the delegate are atomic
    @synchronized(_metalLayer)
    {
        if(newSize.width == _metalLayer.drawableSize.width &&
           newSize.height == _metalLayer.drawableSize.height)
        {
            return;
        }
        
        _metalLayer.drawableSize = newSize;
        
        [_delegate drawableResize:newSize];
    }

}

- (void)viewDidChangeBackingProperties
{
    [super viewDidChangeBackingProperties];
    [self resizeDrawable:self.window.screen.backingScaleFactor];
}

- (void)setFrameSize:(NSSize)size
{
    [super setFrameSize:size];
    [self resizeDrawable:self.window.screen.backingScaleFactor];
}

- (void)setBoundsSize:(NSSize)size
{
    [super setBoundsSize:size];
    [self resizeDrawable:self.window.screen.backingScaleFactor];
}


- (void)render
{
    // Must synchronize if rendering on background thread to ensure resize operations from the
    // main thread are complete before rendering which depends on the size occurs.
    @synchronized(_metalLayer) {
        [_delegate renderToMetalLayer:_metalLayer];
    }
}



// IO events


- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return  YES;
}

- (BOOL)resignFirstResponder
{
    NSLog(@"Will resignFirstResponder");
    return [super resignFirstResponder];
}

// Keyboard events

// all these events need to be made thread safe

- (void) keyDown:(NSEvent *)event
{
    [_delegate didReceiveEvent:event];
}

- (void) keyUp:(NSEvent*) event
{
    [_delegate didReceiveEvent:event];
}

-(void) flagsChanged:(NSEvent *)event
{
    [_delegate didReceiveEvent:event];
}

// Mouse events


- (void) updateTrackingAreas {
    
    [super updateTrackingAreas];
    
    for (id area in [self trackingAreas])
        [self removeTrackingArea:area];
    
    NSTrackingAreaOptions options = (NSTrackingActiveAlways |
                                     NSTrackingInVisibleRect |
                                     NSTrackingMouseEnteredAndExited |
                                     NSTrackingMouseMoved);
    
    [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:[self bounds]
                                                       options:options
                                                         owner:self
                                                      userInfo:nil]];

}


-(void) mouseEntered:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) mouseExited:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) mouseMoved:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) mouseUp:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) mouseDown:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) mouseDragged:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) rightMouseUp:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) rightMouseDown:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) rightMouseDragged:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) otherMouseUp:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) otherMouseDown:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) otherMouseDragged:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

-(void) scrollWheel:(NSEvent *)event {
    [_delegate didReceiveEvent:event];
}

@end
