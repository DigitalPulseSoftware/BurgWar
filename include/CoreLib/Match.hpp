// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MATCH_HPP
#define BURGWAR_CORELIB_MATCH_HPP

#include <CoreLib/AssetStore.hpp>
#include <CoreLib/Export.hpp>
#include <CoreLib/Map.hpp>
#include <CoreLib/MasterServerEntry.hpp>
#include <CoreLib/MatchSessions.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/DestructionWatcherComponent.hpp>
#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <CoreLib/Scripting/ServerWeaponStore.hpp>
#include <Nazara/Utils/Bitset.hpp>
#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class BurgApp;
	class Mod;
	class ServerGamemode;
	class ServerScriptingLibrary;
	class Terrain;

	enum class DisconnectionReason
	{
		Kicked,
		PlayerLeft,
		TimedOut
	};

	class BURGWAR_CORELIB_API Match : public SharedMatch
	{
		friend class MatchClientSession;

		public:
			struct ClientAsset;
			struct ClientScript;
			struct GamemodeSettings;
			struct MatchSettings;
			struct ModSettings;

			Match(BurgApp& app, MatchSettings matchSettings, GamemodeSettings gamemodeSettings, ModSettings modSettings);
			Match(const Match&) = delete;
			Match(Match&&) = delete;
			~Match();

			inline EntityId AllocateUniqueId();

			inline void BroadcastChatMessage(std::string message);
			void BroadcastChatMessage(Player* player, std::string message);
			template<typename T> void BroadcastPacket(const T& packet, bool onlyReady = true, Player* except = nullptr);

			void BuildClientAssetListPacket(Packets::MatchData& clientAsset) const;
			void BuildClientScriptListPacket(Packets::MatchData& clientScript) const;

			Player* CreatePlayer(MatchClientSession& session, Nz::UInt8 localIndex, std::string name);

			void ForEachEntity(tl::function_ref<void(entt::handle entity)> func) override;
			template<typename F> void ForEachPlayer(F&& func, bool onlyReady = true);

			inline BurgApp& GetApp();
			inline const std::shared_ptr<Nz::VirtualDirectory>& GetAssetDirectory() const;
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
			inline const Map& GetMap() const;
			inline const Packets::MatchData& GetMatchData() const;
			inline const ModSettings& GetModSettings() const;
			const NetworkStringStore& GetNetworkStringStore() const override;
			inline Player* GetPlayerByIndex(Nz::UInt16 playerIndex);
			inline const std::shared_ptr<Nz::VirtualDirectory>& GetScriptDirectory() const;
			inline const std::shared_ptr<ServerScriptingLibrary>& GetScriptingLibrary() const;
			inline MatchSessions& GetSessions();
			inline const MatchSessions& GetSessions() const;
			inline const MatchSettings& GetSettings() const;
			std::shared_ptr<const SharedGamemode> GetSharedGamemode() const override;
			inline Terrain& GetTerrain();
			inline const Terrain& GetTerrain() const;
			ServerWeaponStore& GetWeaponStore() override;
			const ServerWeaponStore& GetWeaponStore() const override;

			void InitDebugGhosts();

			void RegisterClientAsset(std::string assetPath);
			void RegisterClientScript(std::string scriptPath);
			void RegisterEntity(EntityId uniqueId, entt::handle entity);
			void RegisterNetworkString(std::string string);

			void ReloadAssets();
			void ReloadMods();
			void ReloadScripts();

			void RemovePlayer(Player* player, DisconnectionReason disconnection);
			void ResetTerrain();

			entt::handle RetrieveEntityByUniqueId(EntityId uniqueId) const override;
			EntityId RetrieveUniqueIdByEntity(entt::handle entity) const override;

			bool Update(float elapsedTime);

			inline void Quit();

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
				std::string description;
				Nz::UInt16 port = 0;
				Map map;
				bool sleepWhenEmpty = true;
				bool registerToMasterServer = true;
				float tickDuration;
			};

			struct ModSettings
			{
				struct ModEntry
				{
					PropertyValueMap properties; //< TODO: Not used yet
				};

				tsl::hopscotch_map<std::string, ModEntry> enabledMods;
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
				entt::handle entity;

				NazaraSlot(DestructionWatcherComponent, OnDestruction, onDestruction);
			};

			std::shared_ptr<ScriptingContext> m_scriptingContext; //< Must be over script based classes
			std::optional<AssetStore> m_assetStore;
			std::optional<Debug> m_debug;
			std::optional<ServerEntityStore> m_entityStore;
			std::optional<ServerWeaponStore> m_weaponStore;
			std::size_t m_maxPlayerCount;
			std::shared_ptr<ServerGamemode> m_gamemode;
			std::shared_ptr<ServerScriptingLibrary> m_scriptingLibrary;
			std::shared_ptr<Nz::VirtualDirectory> m_assetDirectory;
			std::shared_ptr<Nz::VirtualDirectory> m_scriptDirectory;
			std::string m_name;
			std::unique_ptr<Terrain> m_terrain;
			std::vector<std::shared_ptr<Mod>> m_enabledMods;
			std::vector<std::unique_ptr<MasterServerEntry>> m_masterServerEntries;
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
			MatchSettings m_settings;
			ModSettings m_modSettings;
			NetworkStringStore m_networkStringStore;
			bool m_isResetting;
			bool m_isMatchRunning;
	};
}

#include <CoreLib/Match.inl>

#endif
