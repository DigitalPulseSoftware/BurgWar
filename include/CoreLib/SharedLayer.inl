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
		m_registry.each([&](entt::entity entity)
		{
			func(entt::handle(m_registry, entity));
		});
	}

	inline LayerIndex SharedLayer::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline SharedMatch& SharedLayer::GetMatch()
	{
		return m_match;
	}

	inline Nz::Physics2DSystem& SharedLayer::GetPhysicsSystem()
	{
		return m_systemGraph.GetSystem<Nz::Physics2DSystem>();
	}
	
	inline entt::registry& SharedLayer::GetWorld()
	{
		return m_registry;
	}

	inline const entt::registry& SharedLayer::GetWorld() const
	{
		return m_registry;
	}

	inline Nz::EnttSystemGraph& SharedLayer::GetSystemGraph()
	{
		return m_systemGraph;
	}
}
