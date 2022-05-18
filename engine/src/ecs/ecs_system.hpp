#pragma once

#include <set>

#include "ecs_types.hpp"

namespace engine
{
    namespace ecs
    {
		class ECSSystem
		{
			public:
				std::set<ECSEntity> entities_;
		};
	}
}