// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Match.hpp>
#include <cassert>

namespace bw
{
	template<typename F>
	void Match::ForEachPlayer(F&& func)
	{
		for (Player* player : m_players)
		{
			if (player)
				func(player);
		}
	}

	inline BurgApp& Match::GetApp()
	{
		return m_app;
	}

	inline AssetStore& Match::GetAssetStore()
	{
		assert(m_assetStore);
		return *m_assetStore;
	}

	inline const std::shared_ptr<ServerGamemode>& Match::GetGamemode()
	{
		return m_gamemode;
	}

	inline const std::filesystem::path& Match::GetGamemodePath() const
	{
		return m_gamemodePath;
	}

	inline sol::state& Match::GetLuaState()
	{
		return m_scriptingContext->GetLuaState();
	}

	inline const NetworkStringStore& Match::GetNetworkStringStore() const
	{
		return m_networkStringStore;
	}

	inline MatchSessions& Match::GetSessions()
	{
		return m_sessions;
	}

	inline const MatchSessions& Match::GetSessions() const
	{
		return m_sessions;
	}

	inline const std::shared_ptr<ServerScriptingLibrary>& Match::GetScriptingLibrary() const
	{
		return m_scriptingLibrary;
	}

	inline Terrain& Match::GetTerrain()
	{
		assert(m_terrain);
		return *m_terrain;
	}

	inline const Terrain& Match::GetTerrain() const
	{
		assert(m_terrain);
		return *m_terrain;
	}
}
