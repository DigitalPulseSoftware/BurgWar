// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedLayer.hpp>
#include <cassert>

namespace bw
{
	inline LayerIndex SharedLayer::GetLayerIndex()
	{
		return m_layerIndex;
	}

	inline SharedMatch& SharedLayer::GetMatch()
	{
		return m_match;
	}
	
	inline Ndk::World& SharedLayer::GetWorld()
	{
		return m_world;
	}

	inline const Ndk::World& SharedLayer::GetWorld() const
	{
		return m_world;
	}
}
