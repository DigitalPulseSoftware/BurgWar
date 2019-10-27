// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/TerrainLayer.hpp>

namespace bw
{
	inline Match& TerrainLayer::GetMatch()
	{
		return static_cast<Match&>(SharedLayer::GetMatch());
	}
}
