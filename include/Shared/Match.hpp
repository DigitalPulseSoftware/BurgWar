// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_MATCH_HPP
#define BURGWAR_SHARED_MATCH_HPP

#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Shared/MatchSessions.hpp>
#include <Shared/Protocol/NetworkStringStore.hpp>
#include <Shared/Scripting/ServerScriptingContext.hpp>
#include <Shared/Scripting/ServerEntityStore.hpp>
#include <Shared/Scripting/ServerWeaponStore.hpp>
#include <hopscotch/hopscotch_map.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class BurgApp;
	class Gamemode;
	class Player;
	class Terrain;

	using PlayerHandle = Nz::ObjectHandle<Player>;

	class Match
	{
		public:
			struct ClientScript;

			Match(BurgApp& app, std::string matchName, const std::string& gamemodeName, std::size_t maxPlayerCount);
			Match(const Match&) = delete;
			Match(Match&&) = delete;
			~Match();

			Packets::ClientScriptList BuildClientFileListPacket() const;

			void Leave(Player* player);

			bool GetClientScript(const std::string& filePath, const ClientScript** clientScriptData);

			inline ServerEntityStore& GetEntityStore();
			inline const ServerEntityStore& GetEntityStore() const;
			inline sol::state& GetLuaState();
			inline const std::shared_ptr<Gamemode>& GetGamemode();
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
			std::optional<ServerEntityStore> m_entityStore;
			std::optional<ServerWeaponStore> m_weaponStore;
			std::size_t m_maxPlayerCount;
			std::shared_ptr<Gamemode> m_gamemode;
			std::shared_ptr<ServerScriptingContext> m_scriptingContext;
			std::string m_name;
			std::unique_ptr<Terrain> m_terrain;
			std::vector<PlayerHandle> m_players;
			tsl::hopscotch_map<std::string, ClientScript> m_clientScripts;
			MatchSessions m_sessions;
			NetworkStringStore m_networkStringStore;
	};
}

#include <Shared/Match.inl>

#endif
