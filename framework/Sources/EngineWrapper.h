#ifdef __cplusplus
#include <engine/engine.hpp>
#include <engine/event/event.hpp>
#include <string.h>
#endif

#include <MetalKit/MetalKit.h>

#ifdef __cplusplus
static uint32_t EVENT_CAMERA_ANGLES = engine::event::CAMERA_ANGLES;
static uint32_t EVENT_CAMERA_ANGLES_PITCH = engine::event::camera_angles::PITCH;
static uint32_t EVENT_CAMERA_ANGLES_YAW = engine::event::camera_angles::YAW;
static uint32_t EVENT_CAMERA_ANGLES_ROLL = engine::event::camera_angles::ROLL;
#endif

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
- (void)sendCameraEventWithPitchYawRoll:(double) pitch
                                    yaw:(double) yaw
                                   roll:(double) roll;

@end
