//
//  MyOpenGLView.h
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>

@interface MyOpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
    //NSTrackingArea trackingArea;
}

@end
