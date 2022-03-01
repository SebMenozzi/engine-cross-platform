#import <QuartzCore/CAMetalLayer.h>
#import "MetalView.h"

@implementation MetalView
{
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.renderMode = Diligent::MacOSAppBase::RenderMode::Metal;
    }
    return self;
}

- (id)initWithCoder:(NSCoder*)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        self.renderMode = Diligent::MacOSAppBase::RenderMode::Metal;
    }
    return self;
}


- (void) awakeFromNib
{
    [super awakeFromNib];

    // Back the view with a layer created by the makeBackingLayer method.
    self.wantsLayer = YES;

    [self initApp:self];

    CVDisplayLinkRef displayLink;
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    [self setDisplayLink:displayLink];
    CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, (__bridge void*)self);
    CVDisplayLinkStart(displayLink);

    [self setPostsBoundsChangedNotifications:YES];
    [self setPostsFrameChangedNotifications:YES];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(boundsDidChange:) name:NSViewBoundsDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(boundsDidChange:) name:NSViewFrameDidChangeNotification object:self];
}

// Indicates that the view wants to draw using the backing
// layer instead of using drawRect:.
-(BOOL) wantsUpdateLayer
{
    return YES;
}

// Returns a Metal-compatible layer.
+(Class) layerClass
{
    return [CAMetalLayer class];
}

// If the wantsLayer property is set to YES, this method will
// be invoked to return a layer instance.
-(CALayer*) makeBackingLayer
{
    CALayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    return layer;
}

-(void)render
{
    auto* theApp = [self lockApp];
    if (theApp)
    {
        theApp->Update();
        theApp->Render();
        theApp->Present();
    }
    [self unlockApp];
}


- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
    // There is no autorelease pool when this method is called
    // because it will be called from a background thread.
    // It's important to create one or app can leak objects.
    @autoreleasepool {
        [self render];
    }
    return kCVReturnSuccess;
}

// Rendering loop callback function for use with a CVDisplayLink.
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
                                    const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut,
                                    void* target)
{
    MetalView* view = (__bridge MetalView*)target;
    CVReturn result = [view getFrameForTime:outputTime];
    return result;
}

-(void)boundsDidChange:(NSNotification *)notification
{
    // It is not clear what the proper way to handle window resize is.
    // Cube demo from MoltenVK ignores any window resize notifications and
    // recreates the swap chain if Present or AcquireNextImage fails, causing
    // jagged transitions.

    // According to this thread, there is no solution for flickering during
    // resize in Metal:
    // https://forums.developer.apple.com/thread/77901

    // Calling WindowResize() causes flickering.
    //   Even if [self render] is called ater WindowResize()
    //   Similar results when using Metal kit view
    // Calling [self render] alone produces jagged transitions but no flickering.
    // Calling nothing causes the app to crash during resize.

    NSRect viewRectPoints = [self bounds];
    NSRect viewRectPixels = [self convertRectToBacking:viewRectPoints];
    auto* theApp = [self lockApp];
    if (theApp)
    {
        theApp->WindowResize(viewRectPixels.size.width, viewRectPixels.size.height);
    }
    [self unlockApp];
}

@end
