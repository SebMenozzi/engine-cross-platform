#pragma once

#include "GraphicsEngineInterface.hpp"

namespace Diligent
{
    class GraphicsEngineMacOS : public GraphicsEngineInterface
    {
        public:
            virtual void initialize(void* view){};
    };

}
