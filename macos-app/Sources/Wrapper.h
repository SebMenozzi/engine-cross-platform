#ifdef __cplusplus
#include <Engine/CrossPlatformGraphicsEngine.hpp>
#endif

#import <Cocoa/Cocoa.h>

@interface Wrapper: NSObject

#ifdef __cplusplus
@property Diligent::GraphicsEngineMacOS* diligentApp_;
#endif

// MARK: - Public

- (void)createApplication;
- (void)initialize:(NSView*) view;
- (void)start;
- (void)stop;

@end
