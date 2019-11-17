// Copyright (C) 2019 Jérôme Leclercq
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

		for (auto it = m_serverEntityIdToClient.begin(); it != m_serverEntityIdToClient.end(); ++it)
			func(it.value());
	}

	inline const Nz::Color& LocalLayer::GetBackgroundColor() const
	{
		return m_backgroundColor;
	}

	inline std::optional<std::reference_wrapper<LocalLayerEntity>> LocalLayer::GetEntity(Nz::UInt32 serverId)
	{
		assert(m_isEnabled);

		auto it = m_serverEntityIdToClient.find(serverId);
		if (it == m_serverEntityIdToClient.end())
			return std::nullopt;

		return it.value();
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
