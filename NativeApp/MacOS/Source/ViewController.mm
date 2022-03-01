#import "ViewController.h"
#import "ViewBase.h"

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                                                  owner:self
                                                               userInfo:nil];
    [self.view addTrackingArea:trackingArea];
}

- (void)handleEvent : (NSEvent *)theEvent {
    auto* view = (ViewBase*)self.view;
    auto* theApp = [view lockApp];
    if(theApp){
        theApp->HandleOSXEvent((__bridge void*)theEvent, (__bridge void*)view);
    }
    [view unlockApp];
}


- (void)mouseDown:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
    [self handleEvent:theEvent];
}

- (void)keyEvent:(NSEvent *)theEvent isKeyPressed:(bool)keyPressed
{
    [self handleEvent:theEvent];
}

- (void)keyDown:(NSEvent *)theEvent
{
    [self handleEvent:theEvent];

    [super keyDown:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent
{
    [self handleEvent:theEvent];

    [super keyUp:theEvent];
}

// Informs the receiver that the user has pressed or released a
// modifier key (Shift, Control, and so on)
- (void)flagsChanged:(NSEvent *)event
{
    [self handleEvent:event];

    [super flagsChanged:event];
}

- (void)scrollWheel:(NSEvent *)event
{
    [self handleEvent:event];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end
