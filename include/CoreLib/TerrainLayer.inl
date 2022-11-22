// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/TerrainLayer.hpp>

namespace bw
{
	inline NetworkSyncSystem& TerrainLayer::GetNetworkSyncSystem()
	{
		return GetSystemGraph().GetSystem<NetworkSyncSystem>();
	}
}
