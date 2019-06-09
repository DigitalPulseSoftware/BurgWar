// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatchPrediction.hpp>

namespace bw
{
	inline void LocalMatchPrediction::DeleteEntity(Ndk::EntityId entityId)
	{
		m_entities.erase(entityId);
	}

	inline void LocalMatchPrediction::DeleteUnregisteredEntities()
	{
		for (auto it = m_entities.begin(); it != m_entities.end();)
		{
			if (m_registeredEntities.UnboundedTest(it->first))
				++it;
			else
				it = m_entities.erase(it);
		}

		m_registeredEntities.Clear();
	}

	inline const Ndk::EntityHandle& LocalMatchPrediction::GetEntity(Ndk::EntityId entityId)
	{
		auto it = m_entities.find(entityId);
		if (it != m_entities.end())
			return it->second;

		return Ndk::EntityHandle::InvalidHandle;
	}

	inline bool LocalMatchPrediction::IsRegistered(Ndk::EntityId entityId) const
	{
		return m_registeredEntities.UnboundedTest(entityId);
	}

	inline void LocalMatchPrediction::RegisterForPrediction(const Ndk::EntityHandle& entity)
	{
		return RegisterForPrediction(entity, SynchronizeEntity);
	}

	inline void LocalMatchPrediction::RegisterForPrediction(const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor)
	{
		return RegisterForPrediction(entity, constructor, SynchronizeEntity);
	}

	inline void LocalMatchPrediction::Tick()
	{
		m_world.Update(m_tickDuration);
	}
}