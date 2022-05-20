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
            [window setBackgroundColor: NSColor.blackColor];

            MTKView* mtkView = [[MTKView alloc] initWithFrame: window.contentView.bounds];
            mtkView.device = MTLCreateSystemDefaultDevice();
            mtkView.translatesAutoresizingMaskIntoConstraints = false;
            mtkView.wantsLayer = true;

            [window.contentView addSubview:mtkView];

            [mtkView.leadingAnchor constraintEqualToAnchor:window.contentView.leadingAnchor].active = YES;
            [mtkView.trailingAnchor constraintEqualToAnchor:window.contentView.trailingAnchor].active = YES;
            [mtkView.topAnchor constraintEqualToAnchor:window.contentView.topAnchor].active = YES;
            [mtkView.bottomAnchor constraintEqualToAnchor:window.contentView.bottomAnchor].active = YES;

            return mtkView;
        }
    #else
        void* make_native_metal_view(void* native_window_handle) { return nullptr; }
    #endif
#endif
