#import "ModeSelectionViewController.h"
#import "GLView.h"
#import "MetalView.h"
#import "ViewController.h"

@implementation ModeSelectionViewController
{
}

- (void) setWindowTitle:(NSString*) title
{
    NSWindow* mainWindow = [[NSApplication sharedApplication]mainWindow];
    [mainWindow setTitle:title];
}

- (void)viewDidLoad
{
#if !GL_SUPPORTED && !GLES_SUPPORTED
    ((NSButton*)self.view.subviews[0]).enabled = false;
#endif

#if !VULKAN_SUPPORTED
    ((NSButton*)self.view.subviews[1]).enabled = false;
#endif

#if !METAL_SUPPORTED
    ((NSButton*)self.view.subviews[2]).enabled = false;
#endif
}

- (void) terminateApp:(NSString*) error
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Failed to start the application"];
    [alert setInformativeText:error];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];
    [NSApp terminate:self];
}

- (IBAction)goOpenGL:(id)sender
{
    ViewController* glViewController = [self.storyboard instantiateControllerWithIdentifier:@"GLViewControllerID"];
    self.view.window.contentViewController = glViewController;

    GLView* glView = (GLView*)[glViewController view];
    NSString* error = [glView getError];
    if(error != nil)
    {
        [self terminateApp:error];
    }

    NSString* name =  [glView getAppName];
    [self setWindowTitle:name];
}

- (IBAction)goVulkan:(id)sender
{
    ViewController* metalViewController = [self.storyboard instantiateControllerWithIdentifier:@"MoltenVKViewControllerID"];
    self.view.window.contentViewController = metalViewController;

    MetalView* mtlView = (MetalView*)[metalViewController view];
    NSString* error = [mtlView getError];
    if(error != nil)
    {
        [self terminateApp:error];
    }

    NSString* name =  [mtlView getAppName];
    [self setWindowTitle:name];
}

- (IBAction)goMetal:(id)sender
{
    ViewController* metalViewController = [self.storyboard instantiateControllerWithIdentifier:@"MetalViewControllerID"];
    MetalView* mtlView = (MetalView*)[metalViewController view];
    self.view.window.contentViewController = metalViewController;

    NSString* error = [mtlView getError];
    if(error != nil)
    {
        [self terminateApp:error];
    }

    NSString* name =  [mtlView getAppName];
    [self setWindowTitle:name];
}

@end
