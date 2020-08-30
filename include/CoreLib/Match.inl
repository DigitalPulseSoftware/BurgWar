// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Match.hpp>
#include <cassert>

namespace bw
{
	inline Nz::Int64 Match::AllocateUniqueId()
	{
		return m_nextUniqueId++;
	}

	template<typename T>
	void Match::BroadcastPacket(const T& packet, bool onlyReady)
	{
		ForEachPlayer([&packet, onlyReady](Player* player)
		{
			if (!onlyReady || player->IsReady())
				player->SendPacket(packet);
		});
	}

	template<typename T>
	void Match::BuildClientAssetListPacket(T& clientAsset) const
	{
		//FIXME
		clientAsset.fastDownloadUrls.emplace_back("https://burgwar.digitalpulsesoftware.net/resources");

		for (const auto& pair : m_assets)
		{
			auto& assetData = clientAsset.assets.emplace_back();
			assetData.path = pair.second.path;
			assetData.size = pair.second.size;

			const Nz::ByteArray& checksum = pair.second.checksum;
			assert(assetData.sha1Checksum.size() == checksum.size());
			std::memcpy(assetData.sha1Checksum.data(), checksum.GetConstBuffer(), checksum.GetSize());
		}

		std::sort(clientAsset.assets.begin(), clientAsset.assets.end(), [](const auto& first, const auto& second) { return first.path < second.path; });
	}

	template<typename T>
	void Match::BuildClientScriptListPacket(T& clientScript) const
	{
		for (const auto& pair : m_clientScripts)
		{
			auto& scriptData = clientScript.scripts.emplace_back();
			scriptData.path = pair.first;

			const Nz::ByteArray& checksum = pair.second.checksum;
			assert(scriptData.sha1Checksum.size() == checksum.size());
			std::memcpy(scriptData.sha1Checksum.data(), checksum.GetConstBuffer(), checksum.GetSize());
		}

		std::sort(clientScript.scripts.begin(), clientScript.scripts.end(), [](const auto& first, const auto& second) { return first.path < second.path; });
	}

	template<typename F>
	void Match::ForEachPlayer(F&& func)
	{
		for (auto& playerPtr : m_players)
		{
			if (playerPtr)
				func(playerPtr.get());
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

	inline sol::state& Match::GetLuaState()
	{
		return m_scriptingContext->GetLuaState();
	}

	inline const Packets::MatchData& Match::GetMatchData() const
	{
		m_matchData.currentTick = GetNetworkTick();
		return m_matchData;
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
