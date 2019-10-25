//
//  AppDelegate.m
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    NSLog(@"applicationWillResignActive");
    // Insert code here to clear cached key states?
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    NSLog(@"applicationDidResignActive");
}



@end
