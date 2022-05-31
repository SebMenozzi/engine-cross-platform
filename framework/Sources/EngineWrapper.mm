#include "EngineWrapper.h"

#pragma mark - Private Declarations

@implementation EngineWrapper

// MARK: - Public

- (void)create {
    self.engine_ = new engine::Engine();
}

- (void)initializeViewAndAssetsPath:(MTKView*) view
                               path:(NSString*) path {
    Diligent::NativeWindow native_window;
    native_window.pCALayer = (__bridge void*) view.layer;

    const std::string& str = std::string([path UTF8String]);
    
    self.engine_->init(native_window, str);
}

- (void)update:(double) dt {
    // There is no autorelease pool when this method is called
    // because it will be called from a background thread.
    // It's important to create one or app can leak objects.
    @autoreleasepool {
        self.engine_->update(dt);
    }
}

- (void)shutdown {
    self.engine_->shutdown();
}

- (void)sendCameraEventWithPitchYawRoll:(double) pitch
                                    yaw:(double) yaw
                                   roll:(double) roll {
    engine::event::Event event(EVENT_CAMERA_ANGLES);
    event.set_parameter(EVENT_CAMERA_ANGLES_PITCH, pitch);
    event.set_parameter(EVENT_CAMERA_ANGLES_YAW, yaw);
    event.set_parameter(EVENT_CAMERA_ANGLES_ROLL, roll);
    
    self.engine_->send_event(event);
}

@end
