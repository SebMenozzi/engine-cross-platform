#include "EngineWrapper.h"

#pragma mark - Private Declarations

@implementation EngineWrapper

// MARK: - Public

- (void)create {
    self.engine_ = new engine::Engine();
}

- (void)initialize:(MTKView*) view {
    Diligent::NativeWindow native_window;
    native_window.pCALayer = (__bridge void*) view.layer;
    
    self.engine_->init(native_window);
}

- (void)update:(double) dt {
    self.engine_->update(dt);
}

- (void)shutdown {
    self.engine_->shutdown();
}

@end
