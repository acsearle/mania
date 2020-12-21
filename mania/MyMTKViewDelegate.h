//
//  MyMTKViewDelegate.h
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef MyMTKViewDelegate_h
#define MyMTKViewDelegate_h

@import MetalKit;

@interface MyMTKViewDelegate : NSObject<MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;

@end

#endif /* MyMTKViewDelegate_h */
