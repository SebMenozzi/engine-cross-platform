#pragma once

namespace Diligent
{
    class GraphicsEngineInterface
    {
        public:
            virtual void start() = 0;
            virtual void stop() = 0;
    };
}
