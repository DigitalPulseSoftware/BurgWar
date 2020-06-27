// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/NetworkSyncSystem.hpp>

namespace bw
{
	inline TerrainLayer& NetworkSyncSystem::GetLayer()
	{
		return m_layer;
	}

	inline const TerrainLayer& NetworkSyncSystem::GetLayer() const
	{
		return m_layer;
	}
}
