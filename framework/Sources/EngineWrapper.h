#ifdef __cplusplus
#include <engine/engine.hpp>
#include <string.h>
#endif

#include <MetalKit/MetalKit.h>

@interface EngineWrapper: NSObject

#ifdef __cplusplus
@property engine::Engine* engine_;
#endif

// MARK: - Public

- (void)create;
- (void)initializeViewAndAssetsPath:(MTKView*) view 
                               path:(NSString*) path;
- (void)update:(double) dt;
- (void)shutdown;

@end
