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
    self.engine_->update(dt);
}

- (void)shutdown {
    self.engine_->shutdown();
}

@end
