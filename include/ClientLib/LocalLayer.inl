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

		for (auto it = m_serverEntities.begin(); it != m_serverEntities.end(); ++it)
			func(it.value().layerEntity);

		for (auto it = m_clientEntities.begin(); it != m_clientEntities.end(); ++it)
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

	inline std::optional<std::reference_wrapper<LocalLayerEntity>> LocalLayer::GetEntity(Nz::UInt32 serverId)
	{
		assert(m_isEnabled);

		auto it = m_serverEntities.find(serverId);
		if (it == m_serverEntities.end())
			return std::nullopt;

		return it.value().layerEntity;
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
