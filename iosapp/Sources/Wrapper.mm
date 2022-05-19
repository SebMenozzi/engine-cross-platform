#include "Wrapper.h"

#pragma mark - Private Declarations

@implementation Wrapper

// MARK: - Public

- (void)createApplication {
    self.diligentApp_ = Diligent::CreateApplication();
}

- (void)initialize:(NSView*) view {
    self.diligentApp_->initialize((__bridge void*) view);
}

- (void)start {
    self.diligentApp_->start();
}

- (void)stop {
    self.diligentApp_->stop();
}

@end