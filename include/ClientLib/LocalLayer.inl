// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayer.hpp>

namespace bw
{
	inline void LocalLayer::Disable()
	{
		return Enable(false);
	}

	inline void LocalLayer::EnablePrediction(bool enable)
	{
		m_isPredictionEnabled = enable;
	}

	template<typename F>
	void LocalLayer::ForEachLayerEntity(F&& func)
	{
		assert(m_isEnabled);

		for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
			func(it.value().layerEntity);
	}

	template<typename F>
	void LocalLayer::ForEachLayerSound(F&& func)
	{
		assert(m_isEnabled);

		for (auto& soundOpt : m_sounds)
		{
			if (soundOpt)
				func(soundOpt.value().soundIndex, soundOpt.value().sound);
		}
	}

	inline const Nz::Color& LocalLayer::GetBackgroundColor() const
	{
		return m_backgroundColor;
	}

	inline std::optional<std::reference_wrapper<LocalLayerEntity>> LocalLayer::GetEntity(EntityId uniqueId)
	{
		assert(m_isEnabled);

		auto it = m_entities.find(uniqueId);
		if (it == m_entities.end())
			return std::nullopt;

		return it.value().layerEntity;
	}

	inline std::optional<std::reference_wrapper<LocalLayerEntity>> LocalLayer::GetEntityByServerId(Nz::UInt32 serverId)
	{
		assert(m_isEnabled);

		EntityId uniqueId = GetUniqueIdByServerId(serverId);
		if (uniqueId == 0)
			return std::nullopt;

		auto entityOpt = GetEntity(uniqueId);
		assert(entityOpt);

		return entityOpt;
	}

	inline EntityId LocalLayer::GetUniqueIdByServerId(Nz::UInt32 serverId)
	{
		assert(m_isEnabled);

		auto it = m_serverEntityIds.find(serverId);
		if (it == m_serverEntityIds.end())
			return 0;

		return it->second;
	}

	inline bool LocalLayer::IsEnabled() const
	{
		return m_isEnabled;
	}

	inline bool LocalLayer::IsPredictionEnabled() const
	{
		return m_isPredictionEnabled;
	}
}
