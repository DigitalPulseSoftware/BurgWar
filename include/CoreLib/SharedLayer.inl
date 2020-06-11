// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedLayer.hpp>
#include <cassert>

namespace bw
{
	template<typename F>
	void SharedLayer::ForEachEntity(F&& func)
	{
		for (const Ndk::EntityHandle& entity : m_world.GetEntities())
			func(entity);
	}

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
