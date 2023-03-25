// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientMatch.hpp>

namespace bw
{
	template<typename T>
	T ClientMatch::AdjustServerTick(T tick)
	{
		return tick - 3;
	}

	template<typename F>
	void ClientMatch::ForEachPlayer(F&& func)
	{
		for (auto& playerOpt : m_matchPlayers)
		{
			if (!playerOpt)
				continue;

			func(playerOpt.value());
		}
	}

	inline EntityId bw::ClientMatch::AllocateClientUniqueId()
	{
		return m_freeClientId--;
	}

	inline Nz::UInt16 ClientMatch::GetActiveLayer()
	{
		return m_activeLayerIndex;
	}

	inline AnimationManager& ClientMatch::GetAnimationManager()
	{
		return m_animationManager;
	}

	inline ClientAssetStore& ClientMatch::GetAssetStore()
	{
		assert(m_assetStore.has_value());
		return *m_assetStore;
	}

	inline ClientEditorAppComponent& ClientMatch::GetApplication()
	{
		return m_application;
	}

	inline Camera& ClientMatch::GetCamera()
	{
		return *m_camera;
	}

	inline const Camera& ClientMatch::GetCamera() const
	{
		return *m_camera;
	}

	inline ClientSession& ClientMatch::GetClientSession()
	{
		return m_session;
	}

	inline ClientPlayer* ClientMatch::GetLocalPlayerClientPlayer(std::size_t localPlayerIndex)
	{
		assert(localPlayerIndex < m_localPlayers.size());
		Nz::UInt16 playerIndex = m_localPlayers[localPlayerIndex].playerIndex;
		assert(playerIndex < m_matchPlayers.size() && m_matchPlayers[playerIndex].has_value());
		return &m_matchPlayers[playerIndex].value();
	}

	inline const ClientPlayer* ClientMatch::GetLocalPlayerClientPlayer(std::size_t localPlayerIndex) const
	{
		assert(localPlayerIndex < m_localPlayers.size());
		Nz::UInt16 playerIndex = m_localPlayers[localPlayerIndex].playerIndex;
		assert(playerIndex < m_matchPlayers.size() && m_matchPlayers[playerIndex].has_value());
		return &m_matchPlayers[playerIndex].value();
	}

	inline std::size_t ClientMatch::GetLocalPlayerCount() const
	{
		return m_localPlayers.size();
	}

	inline const PlayerInputData& ClientMatch::GetLocalPlayerInputs(std::size_t localPlayerIndex) const
	{
		assert(localPlayerIndex < m_localPlayers.size());
		return m_localPlayers[localPlayerIndex].lastInputData;
	}
	/*
	inline ParticleRegistry& ClientMatch::GetParticleRegistry()
	{
		assert(m_particleRegistry);
		return *m_particleRegistry;
	}

	inline const ParticleRegistry& ClientMatch::GetParticleRegistry() const
	{
		assert(m_particleRegistry);
		return *m_particleRegistry;
	}
	*/
	inline ClientPlayer* ClientMatch::GetPlayerByIndex(Nz::UInt16 playerIndex)
	{
		if (playerIndex >= m_matchPlayers.size() || !m_matchPlayers[playerIndex].has_value())
			return nullptr;

		return &m_matchPlayers[playerIndex].value();
	}

	inline entt::registry& ClientMatch::GetRenderWorld()
	{
		return m_renderWorld.registry;
	}

	inline void ClientMatch::Quit()
	{
		m_isLeavingMatch = true;
	}
}
