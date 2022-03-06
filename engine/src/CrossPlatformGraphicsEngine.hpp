#pragma once

// clang-format off

#include "GraphicsEngineMacOS.hpp"
namespace Diligent
{
    using CrossPlatformGraphicsEngine = GraphicsEngineMacOS;
}

namespace Diligent
{
    extern CrossPlatformGraphicsEngine* CreateApplication();
}
