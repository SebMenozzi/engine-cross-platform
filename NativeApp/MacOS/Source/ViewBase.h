#include "NativeAppBase.hpp"

#import <AppKit/AppKit.h>
#import <QuartzCore/CVDisplayLink.h>
#import <Cocoa/Cocoa.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@interface ViewBase : NSOpenGLView

#pragma clang diagnostic pop

@property CVDisplayLinkRef displayLink;
@property Diligent::MacOSAppBase::RenderMode renderMode;

-(void)initApp:(NSView*) view;
-(void)destroyApp;
-(NSString*)getError;
-(Diligent::NativeAppBase*)lockApp;
-(void)unlockApp;
-(void)stopDisplayLink;
-(void)startDisplayLink;
-(NSString*)getAppName;

@end
