// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MATCH_HPP
#define BURGWAR_CORELIB_MATCH_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <CoreLib/AssetStore.hpp>
#include <CoreLib/Map.hpp>
#include <CoreLib/MatchSessions.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <CoreLib/Scripting/ServerWeaponStore.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class BurgApp;
	class ServerGamemode;
	class ServerScriptingLibrary;
	class Terrain;

	enum class DisconnectionReason
	{
		Kicked,
		PlayerLeft,
		TimedOut
	};

	class Match : public SharedMatch
	{
		friend class MatchClientSession;

		public:
			struct ClientAsset;
			struct ClientScript;
			struct MatchSettings;
			struct GamemodeSettings;

			Match(BurgApp& app, MatchSettings matchSettings, GamemodeSettings gamemodeSettings);
			Match(const Match&) = delete;
			Match(Match&&) = delete;
			~Match();

			inline EntityId AllocateUniqueId();

			void BroadcastChatMessage(Player* player, std::string message);
			template<typename T> void BroadcastPacket(const T& packet, bool onlyReady = true);

			void BuildClientAssetListPacket(Packets::MatchData& clientAsset) const;
			void BuildClientScriptListPacket(Packets::MatchData& clientScript) const;

			Player* CreatePlayer(MatchClientSession& session, Nz::UInt8 localIndex, std::string name);

			void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) override;
			template<typename F> void ForEachPlayer(F&& func);

			inline BurgApp& GetApp();
			inline AssetStore& GetAssetStore();
			bool GetClientAsset(const std::string& filePath, const ClientAsset** clientScriptData);
			bool GetClientScript(const std::string& filePath, const ClientScript** clientScriptData);
			ServerEntityStore& GetEntityStore() override;
			const ServerEntityStore& GetEntityStore() const override;
			inline const std::shared_ptr<ServerGamemode>& GetGamemode();
			TerrainLayer& GetLayer(LayerIndex layerIndex) override;
			const TerrainLayer& GetLayer(LayerIndex layerIndex) const override;
			LayerIndex GetLayerCount() const override;
			inline sol::state& GetLuaState();
			inline const Packets::MatchData& GetMatchData() const;
			const NetworkStringStore& GetNetworkStringStore() const override;
			inline MatchSessions& GetSessions();
			inline const MatchSessions& GetSessions() const;
			inline const std::shared_ptr<ServerScriptingLibrary>& GetScriptingLibrary() const;
			std::shared_ptr<const SharedGamemode> GetSharedGamemode() const override;
			inline Terrain& GetTerrain();
			inline const Terrain& GetTerrain() const;
			ServerWeaponStore& GetWeaponStore() override;
			const ServerWeaponStore& GetWeaponStore() const override;

			void InitDebugGhosts();

			void RegisterClientAsset(std::string assetPath);
			void RegisterClientScript(std::string scriptPath);
			void RegisterEntity(EntityId uniqueId, Ndk::EntityHandle entity);
			void RegisterNetworkString(std::string string);

			void ReloadAssets();
			void ReloadScripts();

			void RemovePlayer(Player* player, DisconnectionReason disconnection);

			const Ndk::EntityHandle& RetrieveEntityByUniqueId(EntityId uniqueId) const override;
			EntityId RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const override;

			void Update(float elapsedTime);

			Match& operator=(const Match&) = delete;
			Match& operator=(Match&&) = delete;

			struct ClientAsset
			{
				Nz::ByteArray checksum;
				Nz::UInt64 size;
				std::filesystem::path realPath;
			};

			struct ClientScript
			{
				Nz::ByteArray checksum;
				std::vector<Nz::UInt8> content;
			};

			struct MatchSettings
			{
				std::size_t maxPlayerCount;
				std::string name;
				Map map;
				float tickDuration;
			};

			struct GamemodeSettings
			{
				std::string name;
				PropertyValueMap properties;
			};

		private:
			void BuildMatchData();
			void OnPlayerReady(Player* player);
			void OnTick(bool lastTick) override;
			void RegisterClientAssetInternal(std::string assetPath, Nz::UInt64 assetSize, Nz::ByteArray assetChecksum, std::filesystem::path realPath);
			void SendPingUpdate();

			struct Debug
			{
				Debug() = default;

				Nz::UdpSocket socket;
				Nz::UInt64 lastBroadcastTime = 0;
			};

			struct Entity
			{
				Ndk::EntityHandle entity;

				NazaraSlot(Ndk::Entity, OnEntityDestruction, onDestruction);
			};

			std::shared_ptr<ScriptingContext> m_scriptingContext; //< Must be over script based classes
			std::optional<AssetStore> m_assetStore;
			std::optional<Debug> m_debug;
			std::optional<ServerEntityStore> m_entityStore;
			std::optional<ServerWeaponStore> m_weaponStore;
			std::size_t m_maxPlayerCount;
			std::shared_ptr<ServerGamemode> m_gamemode;
			std::shared_ptr<ServerScriptingLibrary> m_scriptingLibrary;
			std::string m_name;
			std::unique_ptr<Terrain> m_terrain;
			std::vector<std::unique_ptr<Player>> m_players;
			mutable Packets::MatchData m_matchData;
			tsl::hopscotch_map<std::string, ClientAsset> m_clientAssets;
			tsl::hopscotch_map<std::string, ClientScript> m_clientScripts;
			tsl::hopscotch_map<EntityId, Entity> m_entitiesByUniqueId;
			Nz::Bitset<> m_freePlayerId;
			EntityId m_nextUniqueId;
			Nz::UInt64 m_lastPingUpdate;
			BurgApp& m_app;
			GamemodeSettings m_gamemodeSettings;
			Map m_map;
			MatchSessions m_sessions;
			NetworkStringStore m_networkStringStore;
			bool m_disableWhenEmpty;
	};
}

#include <CoreLib/Match.inl>

#endif
