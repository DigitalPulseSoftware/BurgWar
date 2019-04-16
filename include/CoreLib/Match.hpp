// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MATCH_HPP
#define BURGWAR_CORELIB_MATCH_HPP

#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <CoreLib/MatchSessions.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <CoreLib/Scripting/ServerWeaponStore.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class BurgApp;
	class Player;
	class ServerGamemode;
	class Terrain;

	using PlayerHandle = Nz::ObjectHandle<Player>;

	class Match : public SharedMatch
	{
		public:
			struct ClientScript;

			Match(BurgApp& app, std::string matchName, const std::string& gamemodeFolder, std::size_t maxPlayerCount);
			Match(const Match&) = delete;
			Match(Match&&) = delete;
			~Match();

			Packets::ClientScriptList BuildClientFileListPacket() const;

			void Leave(Player* player);

			bool GetClientScript(const std::string& filePath, const ClientScript** clientScriptData);

			inline BurgApp& GetApp();
			inline ServerEntityStore& GetEntityStore();
			inline const ServerEntityStore& GetEntityStore() const;
			inline sol::state& GetLuaState();
			inline const std::shared_ptr<ServerGamemode>& GetGamemode();
			inline const std::filesystem::path& GetGamemodePath() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;
			inline MatchSessions& GetSessions();
			inline const MatchSessions& GetSessions() const;
			inline Terrain& GetTerrain();
			inline const Terrain& GetTerrain() const;
			inline ServerWeaponStore& GetWeaponStore();
			inline const ServerWeaponStore& GetWeaponStore() const;

			bool Join(Player* player);

			void RegisterClientScript(const std::filesystem::path& clientScript);

			void Update(float elapsedTime);

			Match& operator=(const Match&) = delete;
			Match& operator=(Match&&) = delete;

			struct ClientScript
			{
				Nz::ByteArray checksum;
				std::vector<Nz::UInt8> content;
			};

		private:
			std::filesystem::path m_gamemodePath;
			std::optional<ServerEntityStore> m_entityStore;
			std::optional<ServerWeaponStore> m_weaponStore;
			std::size_t m_maxPlayerCount;
			std::shared_ptr<ServerGamemode> m_gamemode;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::string m_name;
			std::unique_ptr<Terrain> m_terrain;
			std::vector<PlayerHandle> m_players;
			tsl::hopscotch_map<std::string, ClientScript> m_clientScripts;
			BurgApp& m_app;
			MatchSessions m_sessions;
			NetworkStringStore m_networkStringStore;
	};
}

#include <CoreLib/Match.inl>

#endif
