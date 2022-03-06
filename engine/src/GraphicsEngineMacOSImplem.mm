#include <queue>
#include <mutex>
#import <Cocoa/Cocoa.h>

#include "CrossPlatformGraphicsEngine.hpp"
#include "GraphicsEngine.hpp"

namespace Diligent
{
    class GraphicsEngineMacOSImplem final : public GraphicsEngine
    {
        public:
            virtual void initialize(void* view) override final
            {
                MacOSNativeWindow window{view};

                GraphicsEngine::initialize_diligent_engine(&window);

                // In Metal, FinishFrame must be called from the same thread
                // that issued rendering commands. On MacOS, however, rendering
                // happens in DisplayLinkCallback which is called from some other
                // thread. To avoid issues with autorelease pool, we have to pop
                // it now by calling FinishFrame.
                GraphicsEngine::context_->Flush();
                GraphicsEngine::context_->FinishFrame();
            }
    };

    CrossPlatformGraphicsEngine* CreateApplication()
    {
        return new GraphicsEngineMacOSImplem;
    }
}
