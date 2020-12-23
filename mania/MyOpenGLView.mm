//
//  MyOpenGLView.mm
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#import "MyOpenGLView.h"

#include <fstream>
#include <iterator>
#include <OpenGL/gl3.h>

#include "application.hpp"
#include "renderer.hpp"

using manic::application;
using manic::string;
using manic::string_view;

string path_for_resource(string_view name, string_view ext) {
    return [[[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:string(name).c_str()]
                                            ofType:[NSString stringWithUTF8String:string(ext).c_str()]] UTF8String];
}

string load(string name) {
    std::ifstream ifs(name.c_str());
    std::string s{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    return string(s.c_str());
}

string manic::load(string_view name, string_view ext) {
    return ::load(path_for_resource(name, ext));
}

@implementation MyOpenGLView {
    // std::unique_ptr<renderer> _renderer;
}

- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime {
    @autoreleasepool {
        [self drawView];
    }
    return kCVReturnSuccess;
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp* now,
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags* flagsOut,
                                      void* displayLinkContext)
{
    CVReturn result = [(__bridge MyOpenGLView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}


- (void) awakeFromNib
{
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
//        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        0
    };
    
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    if (!pf) {
        NSLog(@"No OpenGL pixel format");
    }
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    
    // When we're using a CoreProfile context, crash if we call a legacy OpenGL function
    // This will make it much more obvious where and when such a function call is made so
    // that we can remove such calls.
    // Without this we'd simply get GL_INVALID_OPERATION error for calling legacy functions
    // but it would be more difficult to see where that function was called.
    CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
    
    [self setPixelFormat:pf];
    
    [self setOpenGLContext:context];
    
    // Opt-In to Retina resolution
    [self setWantsBestResolutionOpenGLSurface:YES];
}

- (void) updateTrackingAreas {
    
    /*
    NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect |
                                     NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
    
    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                        options:options
                                                          owner:self
                                                       userInfo:nil];
     */
    
    [super updateTrackingAreas];
    
    while (NSTrackingArea* p = [[self trackingAreas] firstObject]) {
        NSLog(@"Removed tracking");
        [self removeTrackingArea:p];
    }
    
    [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:[self bounds] options:(NSTrackingActiveAlways | NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved) owner:self userInfo:nil]];
    
}

- (void) prepareOpenGL
{
    [super prepareOpenGL];
    
    // Make all the OpenGL calls to setup rendering
    //  and build the necessary rendering objects
    [self initGL];
    
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void*)self);
    
    // Set the display link for the current renderer
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
    // Activate the display link
    CVDisplayLinkStart(displayLink);
    
    // Register to be notified when the window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:[self window]];
}

- (void) windowWillClose:(NSNotification*)notification
{
    // Stop the display link when the window is closing because default
    // OpenGL render buffers will be destroyed.  If display link continues to
    // fire without renderbuffers, OpenGL draw calls will set errors.
    
    CVDisplayLinkStop(displayLink);
}

- (void) initGL
{
    // The reshape function may have changed the thread to which our OpenGL
    // context is attached before prepareOpenGL and initGL are called.  So call
    // makeCurrentContext to ensure that our OpenGL context current to this
    // thread (i.e. makeCurrentContext directs all OpenGL calls on this thread
    // to [self openGLContext])
    [[self openGLContext] makeCurrentContext];
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];
        
    // Init our renderer.  Use 0 for the defaultFBO which is appropriate for
    // OSX (but not iOS since iOS apps must create their own FBO)
    // _renderer = renderer::make();
    application::get();
        
}

- (void)reshape
{
    [super reshape];
    
    // We draw on a secondary thread through the display link. However, when
    // resizing the view, -drawRect is called on the main thread.
    // Add a mutex around to avoid the threads accessing the context
    // simultaneously when resizing.
    CGLLockContext([[self openGLContext] CGLContextObj]);
    
    // Get the view size in Points
    NSRect viewRectPoints = [self bounds];
    
    // Any calculations the renderer does which use pixel dimentions, must be
    // in "retina" space.  [NSView convertRectToBacking] converts point sizes
    // to pixel sizes.  Thus the renderer gets the size in pixels, not points,
    // so that it can set it's viewport and perform and other pixel based
    // calculations appropriately.
    // viewRectPixels will be larger than viewRectPoints for retina displays.
    // viewRectPixels will be the same as viewRectPoints for non-retina displays
    NSRect viewRectPixels = [self convertRectToBacking:viewRectPoints];
    
    // Set the new dimensions in our renderer
    application::get().resize(viewRectPixels.size.width,
                              viewRectPixels.size.height);
    
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void) drawRect: (NSRect) theRect
{
    // Called during resize operations
    
    // Avoid flickering during resize by drawiing
    [self drawView];
}

- (void) drawView
{
    [[self openGLContext] makeCurrentContext];
    
    // We draw on a secondary thread through the display link
    // When resizing the view, -reshape is called automatically on the main
    // thread. Add a mutex around to avoid the threads accessing the context
    // simultaneously when resizing
    CGLLockContext([[self openGLContext] CGLContextObj]);
    
    /*
    application::get().draw();
    */
    CGLFlushDrawable([[self openGLContext] CGLContextObj]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void) dealloc
{
    // Stop the display link BEFORE releasing anything in the view
    // otherwise the display link thread may call into the view and crash
    // when it encounters something that has been release
    CVDisplayLinkStop(displayLink);
    
    CVDisplayLinkRelease(displayLink);
}

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

- (void) keyDown:(NSEvent *)event
{
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().key_down([event.charactersIgnoringModifiers characterAtIndex:0]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void) keyUp:(NSEvent*) event
{
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().key_up([event.charactersIgnoringModifiers characterAtIndex:0]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) flagsChanged:(NSEvent *)event
{
    NSLog(@"%lu", (unsigned long)event.modifierFlags);
}

// Mouse events

-(void) mouseEntered:(NSEvent *)event {
    NSLog(@"mouseEntered");
}

-(void) mouseExited:(NSEvent *)event {
    NSLog(@"mouseExited");
}

-(void) mouseMoved:(NSEvent *)event {
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_moved(p.x, p.y);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) mouseUp:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_up([event buttonNumber]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) mouseDown:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_down([event buttonNumber]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) mouseDragged:(NSEvent *)event {
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_moved(p.x, p.y);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) rightMouseUp:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_up([event buttonNumber]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) rightMouseDown:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_down([event buttonNumber]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) rightMouseDragged:(NSEvent *)event {
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_moved(p.x, p.y);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) otherMouseUp:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_up([event buttonNumber]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) otherMouseDown:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_down([event buttonNumber]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) otherMouseDragged:(NSEvent *)event {
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().mouse_moved(p.x, p.y);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

-(void) scrollWheel:(NSEvent *)event {
    CGLLockContext([[self openGLContext] CGLContextObj]);
    application::get().scrolled([event scrollingDeltaX], [event scrollingDeltaY]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}


@end
