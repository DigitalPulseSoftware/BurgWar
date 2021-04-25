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

	inline void NetworkSyncSystem::NotifyPhysicsUpdate(const Ndk::EntityHandle& entity)
	{
		if (m_physicsEntities.Has(entity))
			m_physicsUpdateEntities.Insert(entity);
	}

	inline void NetworkSyncSystem::NotifyMovementUpdate(const Ndk::EntityHandle& entity)
	{
		// Dynamic entities are sent regulary, only send static for now (TODO: Handle teleportation this way)
		if (m_staticEntities.Has(entity))
			m_movedStaticEntities.Insert(entity);
	}

	inline void NetworkSyncSystem::NotifyScaleUpdate(const Ndk::EntityHandle& entity)
	{
		m_scaleUpdateEntities.Insert(entity);
	}
}
