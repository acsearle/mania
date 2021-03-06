//
//  MyMetalView.h
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

#include <AppKit/AppKit.h>

// Protocol to provide resize and redraw callbacks to a delegate
@protocol MyMetalViewDelegate <NSObject>

- (void)drawableResize:(CGSize)size;

- (void)renderToMetalLayer:(nonnull CAMetalLayer *)metalLayer;

- (void)didReceiveEvent:(nonnull NSEvent*)event;

// also add UI callbacks

@end


@interface MyMetalView : NSView <CALayerDelegate>

@property (nonatomic, nonnull, readonly) CAMetalLayer *metalLayer;

@property (nonatomic, getter=isPaused) BOOL paused;

@property (nonatomic, nullable) id<MyMetalViewDelegate> delegate;

- (void)initCommon;

- (void)resizeDrawable:(CGFloat)scaleFactor;

- (void)stopRenderLoop;

- (void)render;

@end


