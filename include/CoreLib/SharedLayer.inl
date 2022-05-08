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
		for (entt::entity entity : m_world.GetEntities())
			func(entity);
	}

	inline LayerIndex SharedLayer::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline SharedMatch& SharedLayer::GetMatch()
	{
		return m_match;
	}
	
	inline entt::registry& SharedLayer::GetWorld()
	{
		return m_registry;
	}

	inline const entt::registry& SharedLayer::GetWorld() const
	{
		return m_registry;
	}
}
