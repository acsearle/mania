//
//  MyMetalView.m
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#import "MyMetalView.h"

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
    @synchronized(_metalLayer)
    {
        [_delegate renderToMetalLayer:_metalLayer];
    }
}

@end
