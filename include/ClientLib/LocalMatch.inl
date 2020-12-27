// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	template<typename T>
	T LocalMatch::AdjustServerTick(T tick)
	{
		return tick - 3;
	}

	template<typename F>
	void LocalMatch::ForEachPlayer(F&& func)
	{
		for (auto& playerOpt : m_matchPlayers)
		{
			if (!playerOpt)
				continue;

			func(playerOpt.value());
		}
	}

	inline EntityId bw::LocalMatch::AllocateClientUniqueId()
	{
		return m_freeClientId--;
	}

	inline Nz::UInt16 LocalMatch::GetActiveLayer()
	{
		return m_activeLayerIndex;
	}

	inline AnimationManager& LocalMatch::GetAnimationManager()
	{
		return m_animationManager;
	}

	inline ClientAssetStore& LocalMatch::GetAssetStore()
	{
		assert(m_assetStore.has_value());
		return *m_assetStore;
	}

	inline ClientEditorApp& LocalMatch::GetApplication()
	{
		return m_application;
	}

	inline Camera& LocalMatch::GetCamera()
	{
		return *m_camera;
	}

	inline const Camera& LocalMatch::GetCamera() const
	{
		return *m_camera;
	}

	inline ClientSession& LocalMatch::GetClientSession()
	{
		return m_session;
	}

	inline ParticleRegistry& LocalMatch::GetParticleRegistry()
	{
		assert(m_particleRegistry);
		return *m_particleRegistry;
	}

	inline const ParticleRegistry& LocalMatch::GetParticleRegistry() const
	{
		assert(m_particleRegistry);
		return *m_particleRegistry;
	}

	inline LocalPlayer* LocalMatch::GetPlayerByIndex(Nz::UInt16 playerIndex)
	{
		if (playerIndex >= m_matchPlayers.size() || !m_matchPlayers[playerIndex].has_value())
			return nullptr;

		return &m_matchPlayers[playerIndex].value();
	}

	inline Ndk::World& LocalMatch::GetRenderWorld()
	{
		return m_renderWorld;
	}

	inline void LocalMatch::Quit()
	{
		m_isLeavingMatch = true;
	}
}
