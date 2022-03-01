#include <memory>
#include <string>

#import "ViewBase.h"

@implementation ViewBase
{
    std::unique_ptr<Diligent::NativeAppBase> _theApp;
    std::string _error;
    NSRecursiveLock* appLock;
}

@synthesize displayLink;

// Prepares the receiver for service after it has been loaded
// from an Interface Builder archive, or nib file.
- (void) awakeFromNib
{
    [super awakeFromNib];

    _theApp.reset(Diligent::CreateApplication());

    // [self window] is nil here
    auto* mainWindow = [[NSApplication sharedApplication] mainWindow];
    // Register to be notified when the main window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:mainWindow];
    mainWindow.minSize = NSSize{320, 240};
}

-(void)initApp:(NSView*) view
{
    // Init the application.
    try
    {
        _theApp->Initialize((__bridge void*)view, self.renderMode);
    }
    catch(std::runtime_error &err)
    {
        _error = err.what();
        _theApp.reset();
    }
}

-(Diligent::NativeAppBase*)lockApp
{
    [appLock lock];
    return _theApp.get();
}

-(void)unlockApp
{
    [appLock unlock];
}

- (BOOL)acceptsFirstResponder
{
    return YES; // To make keyboard events work
}

-(void)destroyApp
{
    // Stop the display link BEFORE releasing anything in the view
    // otherwise the display link thread may call into the view and crash
    // when it encounters something that has been released
    if (displayLink)
    {
        CVDisplayLinkStop(displayLink);
    }

    [appLock lock];
    _theApp.reset();
    [appLock unlock];
}

-(void) dealloc
{
    [self destroyApp];

    CVDisplayLinkRelease(displayLink);

    [appLock release];

    [super dealloc];
}

-(NSString*)getError
{
    return _error.empty() ? nil : [NSString stringWithFormat:@"%s", _error.c_str()];
}


- (void)stopDisplayLink
{
    if (displayLink)
    {
        CVDisplayLinkStop(displayLink);
    }
}

- (void)startDisplayLink
{
    if (displayLink)
    {
        CVDisplayLinkStart(displayLink);
    }
}

- (void) windowWillClose:(NSNotification*)notification
{
    [self destroyApp];
}

-(NSString*)getAppName
{
    auto* theApp = [self lockApp];
    auto Title = [NSString stringWithFormat:@"%s", theApp ? theApp->GetAppTitle() : ""];
    [self unlockApp];
    return Title;
}

@end
