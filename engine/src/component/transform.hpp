#pragma once

#include <BasicMath.hpp>

namespace engine
{
    namespace component
    {
		struct Transform
		{
			Diligent::float3 position;
			Diligent::float3 rotation;
			Diligent::float3 scale;
		};
	}
}