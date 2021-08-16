// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Match.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/Mod.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Protocol/CompressedInteger.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Scripting/ServerElementLibrary.hpp>
#include <CoreLib/Scripting/ServerEntityLibrary.hpp>
#include <CoreLib/Scripting/ServerWeaponLibrary.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/File.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <tsl/hopscotch_set.h>
#include <cassert>
#include <fstream>

namespace bw
{
	Match::Match(BurgApp& app, MatchSettings matchSettings, GamemodeSettings gamemodeSettings, ModSettings modSettings) :
	SharedMatch(app, LogSide::Server, matchSettings.name, matchSettings.tickDuration),
	m_maxPlayerCount(matchSettings.maxPlayerCount),
	m_nextUniqueId(matchSettings.map.GetFreeUniqueId()),
	m_lastPingUpdate(0),
	m_app(app),
	m_gamemodeSettings(std::move(gamemodeSettings)),
	m_map(std::move(matchSettings.map)),
	m_sessions(*this),
	m_settings(std::move(matchSettings)),
	m_modSettings(std::move(modSettings)),
	m_disableWhenEmpty(true),
	m_isResetting(false)
	{
		ReloadMods();
		ReloadAssets();
		ReloadScripts();

		m_terrain = std::make_unique<Terrain>(*this, m_map);
		m_terrain->Initialize();

		m_scriptingContext->LoadDirectoryOpt("map/autorun");

		BuildMatchData();

		const std::string& masterServerList = m_app.GetConfig().GetStringValue("ServerSettings.MasterServers");
		SplitStringAny(masterServerList, "\f\n\r\t\v ", [&](const std::string_view& masterServerURI)
		{
			if (!masterServerURI.empty())
				m_masterServerEntries.emplace_back(std::make_unique<MasterServerEntry>(*this, std::string(masterServerURI)));

			return true;
		});

		m_gamemode->ExecuteCallback<GamemodeEvent::Init>();
		m_gamemode->ExecuteCallback<GamemodeEvent::MapInit>();

		bwLog(GetLogger(), LogLevel::Info, "Match initialized");
	}

	Match::~Match()
	{
		// Clear timer manager before scripting context gets deleted
		GetScriptPacketHandlerRegistry().Clear();
		GetTimerManager().Clear();

		m_sessions.Clear();
	}

	void Match::BroadcastChatMessage(Player* player, std::string message)
	{
		Packets::ChatMessage chatPacket;
		chatPacket.content = std::move(message);

		if (player)
			chatPacket.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());

		ForEachPlayer([&](Player* player)
		{
			chatPacket.localIndex = player->GetLocalIndex();

			player->SendPacket(chatPacket);
		});
	}

	void Match::BuildClientAssetListPacket(Packets::MatchData& clientAsset) const
	{
		const std::string& fastDownloadUrls = m_app.GetConfig().GetStringValue("ServerSettings.FastDownloadURLs");

		// Make sure url are only present once
		tsl::hopscotch_set<std::string> urls;
		SplitStringAny(fastDownloadUrls, "\f\n\r\t\v ", [&](const std::string_view& url)
		{
			if (!url.empty())
				urls.emplace(url);

			return true;
		});

		for (auto it = urls.begin(); it != urls.end(); ++it)
			clientAsset.fastDownloadUrls.emplace_back(std::move(it.key()));

		for (const auto& pair : m_clientAssets)
		{
			auto& assetData = clientAsset.assets.emplace_back();
			assetData.path = pair.first;
			assetData.size = pair.second.size;

			const Nz::ByteArray& checksum = pair.second.checksum;
			assert(assetData.sha1Checksum.size() == checksum.size());
			std::memcpy(assetData.sha1Checksum.data(), checksum.GetConstBuffer(), checksum.GetSize());
		}

		std::sort(clientAsset.assets.begin(), clientAsset.assets.end(), [](const auto& first, const auto& second) { return first.path < second.path; });
	}

	void Match::BuildClientScriptListPacket(Packets::MatchData& clientScript) const
	{
		for (const auto& pair : m_clientScripts)
		{
			auto& scriptData = clientScript.scripts.emplace_back();
			scriptData.path = pair.first;
			scriptData.size = pair.second.content.size();

			const Nz::ByteArray& checksum = pair.second.checksum;
			assert(scriptData.sha1Checksum.size() == checksum.size());
			std::memcpy(scriptData.sha1Checksum.data(), checksum.GetConstBuffer(), checksum.GetSize());
		}

		std::sort(clientScript.scripts.begin(), clientScript.scripts.end(), [](const auto& first, const auto& second) { return first.path < second.path; });
	}

	Player* Match::CreatePlayer(MatchClientSession& session, Nz::UInt8 localIndex, std::string name)
	{
		if (m_players.size() >= m_maxPlayerCount)
			return nullptr;

		std::size_t playerIndex = m_freePlayerId.FindFirst();
		if (playerIndex == m_freePlayerId.npos)
		{
			playerIndex = m_freePlayerId.GetSize();
			m_freePlayerId.Resize(playerIndex + 1, false);
			m_players.resize(playerIndex + 1);
		}
		else
			m_freePlayerId.Set(playerIndex, false);

		auto playerPtr = std::make_unique<Player>(*this, session, playerIndex, localIndex, std::move(name));
		Player* player = playerPtr.get();

		m_players[playerIndex] = std::move(playerPtr);

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerConnected>(player->CreateHandle());

		return player;
	}

	void Match::ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func)
	{
		for (LayerIndex i = 0; i < m_terrain->GetLayerCount(); ++i)
		{
			auto& layer = m_terrain->GetLayer(i);
			for (const Ndk::EntityHandle& entity : layer.GetWorld().GetEntities())
				func(entity);
		}
	}

	bool Match::GetClientAsset(const std::string& filePath, const ClientAsset** clientAssetData)
	{
		auto it = m_clientAssets.find(filePath);
		if (it == m_clientAssets.end())
			return false;

		*clientAssetData = &it->second;
		return true;
	}

	bool Match::GetClientScript(const std::string& filePath, const ClientScript** clientScriptData)
	{
		auto it = m_clientScripts.find(filePath);
		if (it == m_clientScripts.end())
			return false;

		*clientScriptData = &it->second;
		return true;
	}

	ServerEntityStore& Match::GetEntityStore()
	{
		assert(m_entityStore);
		return *m_entityStore;
	}

	const ServerEntityStore& Match::GetEntityStore() const
	{
		assert(m_entityStore);
		return *m_entityStore;
	}

	TerrainLayer& Match::GetLayer(LayerIndex layerIndex)
	{
		return m_terrain->GetLayer(layerIndex);
	}

	const TerrainLayer& Match::GetLayer(LayerIndex layerIndex) const
	{
		return m_terrain->GetLayer(layerIndex);
	}

	LayerIndex Match::GetLayerCount() const
	{
		return m_terrain->GetLayerCount();
	}

	const NetworkStringStore& Match::GetNetworkStringStore() const
	{
		return m_networkStringStore;
	}

	std::shared_ptr<const SharedGamemode> Match::GetSharedGamemode() const
	{
		return m_gamemode;
	}

	ServerWeaponStore& Match::GetWeaponStore()
	{
		return *m_weaponStore;
	}

	const ServerWeaponStore& Match::GetWeaponStore() const
	{
		return *m_weaponStore;
	}

	void Match::InitDebugGhosts()
	{
		m_debug.emplace(Debug{}); //< Weird clang bug
		if (m_debug->socket.Create(Nz::NetProtocol_IPv4))
			m_debug->socket.EnableBlocking(false);
		else
		{
			bwLog(GetLogger(), LogLevel::Error, "Failed to create debug socket");
			m_debug.reset();
		}
	}

	void Match::RegisterClientAsset(std::string assetPath)
	{
		if (m_clientAssets.find(assetPath) != m_clientAssets.end())
			return;

		VirtualDirectory::Entry entry;
		if (!m_assetDirectory->GetEntry(assetPath, &entry))
			throw std::runtime_error(assetPath + " is not a file");

		std::vector<Nz::UInt8> content;
		if (!std::holds_alternative<VirtualDirectory::PhysicalFileEntry>(entry))
			throw std::runtime_error(assetPath + " is not a file");

		std::filesystem::path filepath = std::get<VirtualDirectory::PhysicalFileEntry>(entry);

		Nz::UInt64 assetSize = std::filesystem::file_size(filepath);
		Nz::ByteArray assetHash = Nz::File::ComputeHash(Nz::HashType_SHA1, filepath.generic_u8string());

		RegisterClientAssetInternal(std::move(assetPath), assetSize, std::move(assetHash), std::move(filepath));
	}

	void Match::RegisterClientScript(std::string scriptPath)
	{
		if (m_clientScripts.find(scriptPath) != m_clientScripts.end())
			return;

		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(scriptPath, &entry))
			throw std::runtime_error(scriptPath + " is not a file");

		std::vector<Nz::UInt8> content;
		if (std::holds_alternative<VirtualDirectory::FileContentEntry>(entry))
			content = std::get<VirtualDirectory::FileContentEntry>(entry);
		else if (std::holds_alternative<VirtualDirectory::PhysicalFileEntry>(entry))
		{
			const std::filesystem::path& filepath = std::get<VirtualDirectory::PhysicalFileEntry>(entry);

			Nz::File file(filepath.generic_u8string());
			if (!file.Open(Nz::OpenMode_ReadOnly))
				throw std::runtime_error("failed to open " + filepath.generic_u8string());

			content.resize(file.GetSize());
			if (file.Read(content.data(), content.size()) != content.size())
				throw std::runtime_error("failed to read " + filepath.generic_u8string());
		}
		else
			throw std::runtime_error(scriptPath + " is not a file");

		auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
		hash->Begin();
		hash->Append(content.data(), content.size());

		ClientScript clientScriptData;
		clientScriptData.checksum = hash->End();
		clientScriptData.content = std::move(content);

		m_clientScripts.emplace(std::move(scriptPath), std::move(clientScriptData));
	}

	void Match::RegisterEntity(EntityId uniqueId, Ndk::EntityHandle entity)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) == m_entitiesByUniqueId.end());

		Entity& entityData = m_entitiesByUniqueId.emplace(uniqueId, Entity{}).first.value();
		entityData.entity = std::move(entity);
		entityData.onDestruction.Connect(entityData.entity->OnEntityDestruction, [this, uniqueId](Ndk::Entity* entity)
		{
			// Don't trigger the Destroyed event when resetting map
			if (!m_isResetting)
			{
				auto& entityScript = entity->GetComponent<ScriptComponent>();
				entityScript.ExecuteCallback<ElementEvent::Destroyed>();
			}

			m_entitiesByUniqueId.erase(uniqueId);
		});
	}

	void Match::RegisterNetworkString(std::string string)
	{
		if (m_networkStringStore.GetStringIndex(string) == m_networkStringStore.InvalidIndex)
		{
			Nz::UInt32 newStringId = m_networkStringStore.RegisterString(std::move(string));

			// Send the new string to all players, if any
			BroadcastPacket(m_networkStringStore.BuildPacket(newStringId), false);
		}
	}

	void Match::ReloadAssets()
	{
		const std::string& assetDirectory = m_app.GetConfig().GetStringValue("Resources.AssetDirectory");

		m_assetDirectory = std::make_shared<VirtualDirectory>(assetDirectory);
		for (const auto& modPtr : m_enabledMods)
		{
			for (const auto& [assetPath, physicalPath] : modPtr->GetAssets())
				m_assetDirectory->StoreFile(assetPath, physicalPath);
		}

		if (!m_assetStore)
			m_assetStore.emplace(GetLogger(), m_assetDirectory);
		else
		{
			m_assetStore->UpdateAssetDirectory(m_assetDirectory);
			m_assetStore->Clear();
		}

		assert(m_map.IsValid());
		for (const auto& asset : m_map.GetAssets())
		{
			std::filesystem::path assetPath = assetDirectory;
			assetPath /= asset.filepath;

			if (!std::filesystem::is_regular_file(assetPath))
			{
				bwLog(GetLogger(), LogLevel::Error, "Map asset file not found ({0})", asset.filepath);
				continue;
			}

			Nz::UInt64 fileSize = std::filesystem::file_size(assetPath);
			if (fileSize != asset.size)
			{
				bwLog(GetLogger(), LogLevel::Error, "Map asset doesn't match file ({0}): size doesn't match (expected {1}, got {2})", asset.filepath, asset.size, fileSize);
				continue;
			}

			Nz::ByteArray expectedChecksum(asset.sha1Checksum.size(), 0);
			std::memcpy(expectedChecksum.GetBuffer(), asset.sha1Checksum.data(), asset.sha1Checksum.size());

			Nz::ByteArray fileChecksum = Nz::File::ComputeHash(Nz::HashType_SHA1, assetPath.generic_u8string());
			if (fileChecksum != expectedChecksum)
			{
				bwLog(GetLogger(), LogLevel::Error, "Map asset doesn't match file ({0}): checksum doesn't match", asset.filepath, asset.size, fileSize);
				continue;
			}

			RegisterClientAssetInternal(asset.filepath, fileSize, fileChecksum, std::move(assetPath));
		}
	}

	void Match::ReloadMods()
	{
		const auto& loadedMods = m_app.GetMods();
		for (auto&& [modId, modSettings] : m_modSettings.enabledMods)
		{
			auto it = loadedMods.find(modId);
			if (it == loadedMods.end())
			{
				bwLog(GetLogger(), LogLevel::Error, "unknown mod {0}", modId);
				continue;
			}

			m_enabledMods.emplace_back(it->second);
		}
	}

	void Match::ReloadScripts()
	{
		assert(m_assetStore);

		m_clientScripts.clear();

		const std::string& scriptFolder = m_app.GetConfig().GetStringValue("Resources.ScriptDirectory");

		m_scriptDirectory = std::make_shared<VirtualDirectory>(scriptFolder);
		for (const auto& modPtr : m_enabledMods)
		{
			for (const auto& [scriptPath, physicalPath] : modPtr->GetScripts())
				m_scriptDirectory->StoreFile(scriptPath, physicalPath);
		}

		for (const auto& mapScript : m_map.GetScripts())
			m_scriptDirectory->StoreFile(mapScript.filepath, mapScript.content);

		if (!m_scriptingContext)
		{
			if (!m_scriptingLibrary)
				m_scriptingLibrary = std::make_shared<ServerScriptingLibrary>(*this, *m_assetStore);

			m_scriptingContext = std::make_shared<ScriptingContext>(GetLogger(), m_scriptDirectory);
			m_scriptingContext->LoadLibrary(m_scriptingLibrary);
		}
		else
		{
			m_scriptingContext->UpdateScriptDirectory(m_scriptDirectory);
			m_scriptingContext->ReloadLibraries();
		}

		std::shared_ptr<ServerElementLibrary> serverElementLib;

		if (!m_entityStore)
		{
			if (!serverElementLib)
				serverElementLib = std::make_shared<ServerElementLibrary>(GetLogger());

			m_entityStore.emplace(GetLogger(), m_scriptingContext);
			m_entityStore->LoadLibrary(serverElementLib);
			m_entityStore->LoadLibrary(std::make_shared<ServerEntityLibrary>(GetLogger()));
		}
		else
		{
			m_entityStore->ClearElements();
			m_entityStore->ReloadLibraries();
		}

		if (!m_weaponStore)
		{
			if (!serverElementLib)
				serverElementLib = std::make_shared<ServerElementLibrary>(GetLogger());

			m_weaponStore.emplace(GetLogger(), m_scriptingContext);
			m_weaponStore->LoadLibrary(serverElementLib);
			m_weaponStore->LoadLibrary(std::make_shared<ServerWeaponLibrary>(GetLogger(), *this));
		}
		else
		{
			m_weaponStore->ClearElements();
			m_weaponStore->ReloadLibraries();
		}

		m_entityStore->LoadDirectory("entities");
		m_entityStore->Resolve();

		m_weaponStore->LoadDirectory("weapons");
		m_weaponStore->Resolve();

		if (!m_gamemode)
			m_gamemode = std::make_shared<ServerGamemode>(*this, m_scriptingContext, m_gamemodeSettings.name, m_gamemodeSettings.properties);
		else
			m_gamemode->Reload();

		for (auto&& [propertyName, propertyData] : m_gamemode->GetProperties())
		{
			if (propertyData.shared)
				m_networkStringStore.RegisterString(propertyName);
		}

		if (m_terrain)
		{
			ForEachEntity([this](const Ndk::EntityHandle& entity)
			{
				if (entity->HasComponent<ScriptComponent>())
				{
					// Warning: ugly (FIXME)
					m_entityStore->UpdateEntityElement(entity);
					m_weaponStore->UpdateEntityElement(entity);
				}
			});
		}

		m_entityStore->ForEachElement([&](const ScriptedEntity& entity)
		{
			if (entity.isNetworked)
			{
				m_networkStringStore.RegisterString(entity.fullName);

				for (auto&& [propertyName, propertyData] : entity.properties)
				{
					if (propertyData.shared)
						m_networkStringStore.RegisterString(propertyName);
				}
			}
		});

		m_weaponStore->ForEachElement([&](const ScriptedWeapon& weapon)
		{
			m_networkStringStore.RegisterString(weapon.fullName);

			for (auto&& [propertyName, propertyData] : weapon.properties)
			{
				if (propertyData.shared)
					m_networkStringStore.RegisterString(propertyName);
			}
		});
	}

	void Match::RemovePlayer(Player* player, DisconnectionReason disconnectionReason)
	{
		assert(&player->GetMatch() == this);

		auto it = std::find_if(m_players.begin(), m_players.end(), [player](const auto& playerPtr) { return playerPtr.get() == player; });
		assert(it != m_players.end());

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerLeave>(player->CreateHandle());

		Packets::ChatMessage chatPacket;
		chatPacket.content = player->GetName() + " has left";

		switch (disconnectionReason)
		{
			case DisconnectionReason::Kicked:
				chatPacket.content += " (kicked).";
				break;

			case DisconnectionReason::PlayerLeft :
				chatPacket.content += '.';
				break;

			case DisconnectionReason::TimedOut:
				chatPacket.content += " (timed out).";
				break;

			default:
				chatPacket.content += " (unhandled case).";
				break;
		}

		if (player->IsReady())
		{
			Packets::PlayerLeaving leavingPacket;
			leavingPacket.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());

			BroadcastPacket(leavingPacket);
		}

		it->reset();
		m_freePlayerId.Set(std::distance(m_players.begin(), it), true);

		ForEachPlayer([&](Player* player)
		{
			chatPacket.localIndex = player->GetLocalIndex();

			player->SendPacket(chatPacket);
		});
	}

	void Match::ResetTerrain()
	{
		for (auto& playerPtr : m_players)
		{
			if (!playerPtr)
				continue;

			MatchClientVisibility& visibility = playerPtr->GetSession().GetVisibility();
			visibility.ShouldIgnoreEvents(true);
		}

		m_isResetting = true;
		m_nextUniqueId = m_map.GetFreeUniqueId();
		m_terrain->Reset();
		m_isResetting = false;

		for (auto& playerPtr : m_players)
		{
			if (!playerPtr)
				continue;

			MatchClientVisibility& visibility = playerPtr->GetSession().GetVisibility();
			visibility.ShouldIgnoreEvents(false);

			visibility.ResetVisibleEntities();
		}

		m_gamemode->ExecuteCallback<GamemodeEvent::MapInit>();
	}

	const Ndk::EntityHandle& Match::RetrieveEntityByUniqueId(EntityId uniqueId) const
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		if (it == m_entitiesByUniqueId.end())
			return Ndk::EntityHandle::InvalidHandle;

		return it.value().entity;
	}

	EntityId Match::RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const
	{
		if (!entity || !entity->HasComponent<MatchComponent>())
			return InvalidEntityId;

		return entity->GetComponent<MatchComponent>().GetUniqueId();
	}

	void Match::Update(float elapsedTime)
	{
		m_sessions.Poll();

		for (const auto& masterServerEntryPtr : m_masterServerEntries)
			masterServerEntryPtr->Update(elapsedTime);

		if (m_disableWhenEmpty && m_freePlayerId.TestAll())
			return;

		m_scriptingContext->Update();

		SharedMatch::Update(elapsedTime);

		Nz::UInt64 appTime = m_app.GetAppTime();
		if (appTime - m_lastPingUpdate > 1000)
		{
			SendPingUpdate();
			m_lastPingUpdate = appTime;
		}


		if (m_debug && appTime - m_debug->lastBroadcastTime > 1000 / 60)
		{
			m_debug->lastBroadcastTime = m_app.GetAppTime();

			// Send all entities state
			Nz::NetPacket debugPacket(1);

			std::size_t offset = debugPacket.GetStream()->GetCursorPos();

			Nz::UInt32 entityCount = 0;
			debugPacket << entityCount;

			for (LayerIndex i = 0; i < m_terrain->GetLayerCount(); ++i)
			{
				auto& layer = m_terrain->GetLayer(i);
				layer.ForEachEntity([&](const Ndk::EntityHandle& entity)
				{
					if (!entity->HasComponent<Ndk::NodeComponent>() || !entity->HasComponent<NetworkSyncComponent>())
						return;

					auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

					entityCount++;

					CompressedUnsigned<Nz::UInt16> layerId(i);
					CompressedUnsigned<Nz::UInt32> entityId(entity->GetId());
					debugPacket << layerId;
					debugPacket << entityId;

					bool isPhysical = entity->HasComponent<Ndk::PhysicsComponent2D>();

					debugPacket << isPhysical;

					Nz::Vector2f entityPosition;
					Nz::RadianAnglef entityRotation;

					if (isPhysical)
					{
						auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

						entityPosition = entityPhys.GetPosition();
						entityRotation = entityPhys.GetRotation();

						debugPacket << entityPhys.GetVelocity() << entityPhys.GetAngularVelocity();
					}
					else
					{
						entityPosition = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Global));
						entityRotation = AngleFromQuaternion(entityNode.GetRotation(Nz::CoordSys_Global));
					}

					debugPacket << entityPosition << entityRotation;
				});
			}

			debugPacket.GetStream()->SetCursorPos(offset);
			debugPacket << entityCount;

			Nz::IpAddress localAddress = Nz::IpAddress::LoopbackIpV4;
			for (std::size_t i = 0; i < 4; ++i)
			{
				localAddress.SetPort(static_cast<Nz::UInt16>(42000 + i));

				if (!m_debug->socket.SendPacket(localAddress, debugPacket))
					bwLog(GetLogger(), LogLevel::Error, "Failed to send debug packet: {1}", Nz::ErrorToString(m_debug->socket.GetLastError()));
			}
		}
	}

	void Match::BuildMatchData()
	{
		// Send match data
		const Map& mapData = m_terrain->GetMap();

		m_matchData.gamemode = m_gamemodeSettings.name;
		m_matchData.tickDuration = GetTickDuration();

		m_matchData.layers.clear();
		m_matchData.layers.reserve(mapData.GetLayerCount());
		for (std::size_t i = 0; i < mapData.GetLayerCount(); ++i)
		{
			const auto& mapLayer = mapData.GetLayer(LayerIndex(i));

			auto& packetLayer = m_matchData.layers.emplace_back();
			packetLayer.backgroundColor = mapLayer.backgroundColor;
		}

		m_matchData.assets.clear();
		m_matchData.fastDownloadUrls.clear();
		BuildClientAssetListPacket(m_matchData);

		m_matchData.scripts.clear();
		BuildClientScriptListPacket(m_matchData);

		const auto& gamemodePropertyData = m_gamemode->GetProperties();
		for (auto&& [propertyName, propertyValue] : m_gamemodeSettings.properties)
		{
			auto propertyIt = gamemodePropertyData.find(propertyName);
			if (propertyIt == gamemodePropertyData.end())
				continue;

			const ScriptedProperty& scriptedProperty = propertyIt->second;
			if (!scriptedProperty.shared)
				continue;

			auto& propertyData = m_matchData.gamemodeProperties.emplace_back();
			propertyData.name = m_networkStringStore.CheckStringIndex(propertyName);
			propertyData.value = propertyValue;
		}
	}

	void Match::OnPlayerReady(Player* newPlayer)
	{
		if (newPlayer->IsReady())
			return;

		// Send a PlayerJoined packet to everyone
		Packets::PlayerJoined joinedPacket;
		joinedPacket.playerIndex = static_cast<Nz::UInt16>(newPlayer->GetPlayerIndex());
		joinedPacket.playerName = newPlayer->GetName();
		BroadcastPacket(joinedPacket);

		newPlayer->SetReady();

		Packets::ChatMessage chatPacket;
		chatPacket.content = newPlayer->GetName() + " has joined.";

		ForEachPlayer([&](Player* player)
		{
			// Send a PlayerJoined packet to the new player, with everyone
			Packets::PlayerJoined joinedPacket;
			joinedPacket.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
			joinedPacket.playerName = player->GetName();

			// Send player local index
			if (player == newPlayer)
				joinedPacket.localIndex = player->GetLocalIndex();

			newPlayer->SendPacket(joinedPacket);

			chatPacket.localIndex = player->GetLocalIndex();

			player->SendPacket(chatPacket);
		});

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerJoined>(newPlayer->CreateHandle());
		
		// Send a packet for every player associating them with the entity they control
		ForEachPlayer([&](Player* player)
		{
			if (player == newPlayer)
				return;

			const Ndk::EntityHandle& controlledEntity = player->GetControlledEntity();
			if (!controlledEntity)
				return;

			auto& entityMatch = controlledEntity->GetComponent<MatchComponent>();

			Packets::PlayerControlEntity controlledEntityUpdate;
			controlledEntityUpdate.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
			controlledEntityUpdate.controlledEntityId = entityMatch.GetUniqueId();

			newPlayer->SendPacket(controlledEntityUpdate);
		});
	}

	void Match::OnTick(bool lastTick)
	{
		float elapsedTime = GetTickDuration();

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			session->OnTick(elapsedTime);
		});

		ForEachPlayer([&](Player* player)
		{
			player->OnTick(lastTick);
		});

		m_gamemode->ExecuteCallback<GamemodeEvent::Tick>();

		m_terrain->Update(elapsedTime);

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			session->Update(elapsedTime);
		});
	}

	void Match::RegisterClientAssetInternal(std::string assetPath, Nz::UInt64 assetSize, Nz::ByteArray assetChecksum, std::filesystem::path realPath)
	{
		if (auto it = m_clientAssets.find(assetPath); it != m_clientAssets.end())
		{
			const ClientAsset& asset = it->second;
			if (asset.size != assetSize)
			{
				bwLog(GetLogger(), LogLevel::Error, "Asset {1} registered twice and size doesn't match", assetPath);
				return;
			}

			if (asset.checksum != assetChecksum)
			{
				bwLog(GetLogger(), LogLevel::Error, "Asset {1} registered twice and checksum doesn't match", assetPath);
				return;
			}
		}
		else
		{
			ClientAsset asset;
			asset.checksum = std::move(assetChecksum);
			asset.realPath = std::move(realPath);
			asset.size = assetSize;

			m_clientAssets.emplace(std::move(assetPath), std::move(asset));
		}
	}
	
	void Match::SendPingUpdate()
	{
		Packets::PlayerPingUpdate pingUpdate;

		ForEachPlayer([&](Player* player)
		{
			auto& playerData = pingUpdate.players.emplace_back();
			playerData.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
			playerData.ping = static_cast<Nz::UInt16>(player->GetSession().GetPing());
		});

		BroadcastPacket(pingUpdate);
	}
}
