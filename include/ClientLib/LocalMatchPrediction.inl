// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatchPrediction.hpp>

namespace bw
{
	inline void LocalMatchPrediction::DeleteEntity(Nz::UInt64 layerEntityId)
	{
		m_entities.erase(layerEntityId);
	}

	inline void LocalMatchPrediction::DeleteUnregisteredEntities()
	{
		for (auto it = m_entities.begin(); it != m_entities.end();)
		{
			auto& entityData = it.value();
			if (entityData.isRegistered)
			{
				entityData.isRegistered = false;
				++it;
			}
			else
				it = m_entities.erase(it);
		}
	}

	inline const Ndk::EntityHandle& LocalMatchPrediction::GetEntity(Nz::UInt64 layerEntityId)
	{
		auto it = m_entities.find(layerEntityId);
		if (it != m_entities.end())
			return it->second.entity;

		return Ndk::EntityHandle::InvalidHandle;
	}

	inline bool LocalMatchPrediction::IsRegistered(Nz::UInt64 layerEntityId) const
	{
		auto it = m_entities.find(layerEntityId);
		if (it == m_entities.end())
			return false;

		return it->second.isRegistered;
	}

	inline void LocalMatchPrediction::RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity)
	{
		return RegisterForPrediction(layerEntityId, entity, SynchronizeEntity);
	}

	inline void LocalMatchPrediction::RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor)
	{
		return RegisterForPrediction(layerEntityId, entity, constructor, SynchronizeEntity);
	}

	inline void LocalMatchPrediction::Tick()
	{
		m_world.Update(m_tickDuration);
	}
}