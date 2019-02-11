// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/TerrainLayer.hpp>

namespace bw
{
	inline Ndk::World& TerrainLayer::GetWorld()
	{
		return m_world;
	}
}
