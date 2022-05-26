#ifdef __cplusplus
#include <engine/engine.hpp>
#endif

#include <MetalKit/MetalKit.h>

@interface EngineWrapper: NSObject

#ifdef __cplusplus
@property engine::Engine* engine_;
#endif

// MARK: - Public

- (void)create;
- (void)initialize:(MTKView*) view;
- (void)update:(double) dt;
- (void)shutdown;

@end
