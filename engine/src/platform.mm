#import "platform.hpp"

#ifdef PLATFORM_MACOS
    #import <QuartzCore/QuartzCore.h>
    #import <AppKit/AppKit.h>
    #import <objc/runtime.h>
    
    #if METAL_SUPPORTED
        #import <Metal/Metal.h>
        #import <MetalKit/MetalKit.h>

        void* make_native_metal_view(void* native_window_handle)
        {
            NSLog(@"Metal View Supported.");

            NSWindow* window = static_cast<NSWindow*>(native_window_handle);
            [window setBackgroundColor: NSColor.redColor];

            MTKView* mtkView = [[MTKView alloc] initWithFrame: window.contentView.bounds];
            mtkView.device = MTLCreateSystemDefaultDevice();
    
            window.contentView = mtkView;

            return mtkView;
        }
    #else
        void* make_native_metal_view(void* native_window_handle) { return nullptr; }
    #endif
#else
    assert(false);
#endif