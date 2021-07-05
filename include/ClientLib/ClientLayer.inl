// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientLayer.hpp>

namespace bw
{
	inline void ClientLayer::Disable()
	{
		return Enable(false);
	}

	inline void ClientLayer::EnablePrediction(bool enable)
	{
		m_isPredictionEnabled = enable;
	}

	template<typename F>
	void ClientLayer::ForEachLayerEntity(F&& func)
	{
		assert(m_isEnabled);

		for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
			func(it.value().layerEntity);
	}

	template<typename F>
	void ClientLayer::ForEachLayerSound(F&& func)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < m_sounds.size(); ++i)
		{
			auto& soundOpt = m_sounds[i];
			if (soundOpt)
				func(i, soundOpt.value().sound);
		}
	}

	inline const Nz::Color& ClientLayer::GetBackgroundColor() const
	{
		return m_backgroundColor;
	}

	inline std::optional<std::reference_wrapper<ClientLayerEntity>> ClientLayer::GetEntity(EntityId uniqueId)
	{
		assert(m_isEnabled);

		auto it = m_entities.find(uniqueId);
		if (it == m_entities.end())
			return std::nullopt;

		return it.value().layerEntity;
	}

	inline std::optional<std::reference_wrapper<ClientLayerEntity>> ClientLayer::GetEntityByServerId(Nz::UInt32 serverId)
	{
		assert(m_isEnabled);

		EntityId uniqueId = GetUniqueIdByServerId(serverId);
		if (uniqueId == InvalidEntityId)
			return std::nullopt;

		auto entityOpt = GetEntity(uniqueId);
		assert(entityOpt);

		return entityOpt;
	}

	inline EntityId ClientLayer::GetUniqueIdByServerId(Nz::UInt32 serverId)
	{
		assert(m_isEnabled);

		auto it = m_serverEntityIds.find(serverId);
		if (it == m_serverEntityIds.end())
			return InvalidEntityId;

		return it->second;
	}

	inline bool ClientLayer::IsPredictionEnabled() const
	{
		return m_isPredictionEnabled;
	}
}
