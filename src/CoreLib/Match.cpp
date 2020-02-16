// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Match.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Player.hpp>
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
#include <cassert>
#include <fstream>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, std::filesystem::path gamemodeFolder, Map map, std::size_t maxPlayerCount, float tickDuration) :
	SharedMatch(app, LogSide::Server, std::move(matchName), tickDuration),
	m_gamemodePath(std::move(gamemodeFolder)),
	m_maxPlayerCount(maxPlayerCount),
	m_sessions(*this),
	m_nextUniqueId(map.GetFreeUniqueId()),
	m_lastPingUpdate(0),
	m_app(app),
	m_map(std::move(map))
	{
		m_scriptingLibrary = std::make_shared<ServerScriptingLibrary>(*this);

		ReloadAssets();
		ReloadScripts();

		m_terrain = std::make_unique<Terrain>(m_map);
		m_terrain->Initialize(*this);

		BuildMatchData();

		m_gamemode->ExecuteCallback("OnInit");

		bwLog(GetLogger(), LogLevel::Info, "Match initialized");
	}

	Match::~Match()
	{
		// Destroy players before scripting context
		m_sessions.Clear();

		// Clear timer manager before scripting context gets deleted
		GetScriptPacketHandlerRegistry().Clear();
		GetTimerManager().Clear();

		// Release scripts classes before scripting context
		m_entityStore.reset();
		m_weaponStore.reset();
		m_gamemode.reset();
		m_scriptingLibrary.reset();
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
		}

		auto playerPtr = std::make_unique<Player>(*this, session, playerIndex, localIndex, std::move(name));
		Player* player = playerPtr.get();

		m_players.emplace_back(std::move(playerPtr));

		m_gamemode->ExecuteCallback("OnPlayerConnected", player->CreateHandle());

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
		m_debug.emplace();
		if (m_debug->socket.Create(Nz::NetProtocol_IPv4))
			m_debug->socket.EnableBlocking(false);
		else
		{
			bwLog(GetLogger(), LogLevel::Error, "Failed to create debug socket");
			m_debug.reset();
		}
	}

	void Match::RegisterAsset(const std::filesystem::path& assetPath)
	{
		std::string relativePath = assetPath.generic_u8string();

		if (m_assets.find(relativePath) != m_assets.end())
			return;

		const std::string& resourceFolder = m_app.GetConfig().GetStringOption("Assets.ResourceFolder");

		std::string filePath = resourceFolder + "/" + relativePath;
		if (!std::filesystem::is_regular_file(filePath))
			throw std::runtime_error(filePath + " is not a file");

		RegisterAsset(std::move(relativePath), std::filesystem::file_size(filePath), Nz::File::ComputeHash(Nz::HashType_SHA1, filePath));
	}

	void Match::RegisterAsset(std::string assetPath, Nz::UInt64 assetSize, Nz::ByteArray assetChecksum)
	{
		if (auto it = m_assets.find(assetPath); it != m_assets.end())
		{
			const Asset& asset = it->second;
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
			Asset asset;
			asset.checksum = std::move(assetChecksum);
			asset.path = assetPath;
			asset.size = assetSize;

			m_assets.emplace(std::move(assetPath), std::move(asset));
		}
	}

	void Match::RegisterClientScript(const std::filesystem::path& clientScript)
	{
		std::string relativePath = clientScript.generic_u8string();

		if (m_clientScripts.find(relativePath) != m_clientScripts.end())
			return;

		const std::string& scriptFolder = m_app.GetConfig().GetStringOption("Assets.ScriptFolder");

		std::string filePath = scriptFolder + "/" + relativePath;
		if (!std::filesystem::is_regular_file(filePath))
			throw std::runtime_error(filePath + " is not a file");

		Nz::File file(filePath);
		if (!file.Open(Nz::OpenMode_ReadOnly))
			throw std::runtime_error("Failed to open " + filePath);

		std::vector<Nz::UInt8> content(file.GetSize());
		if (file.Read(content.data(), content.size()) != content.size())
			throw std::runtime_error("Failed to read " + filePath);

		auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);
		hash->Begin();
		hash->Append(content.data(), content.size());

		ClientScript clientScriptData;
		clientScriptData.checksum = hash->End();
		clientScriptData.content = std::move(content);

		m_clientScripts.emplace(std::move(relativePath), std::move(clientScriptData));
	}

	void Match::RegisterEntity(Nz::Int64 uniqueId, Ndk::EntityHandle entity)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) == m_entitiesByUniqueId.end());

		Entity& entityData = m_entitiesByUniqueId.emplace(uniqueId, Entity{}).first.value();
		entityData.entity = std::move(entity);
		entityData.onDestruction.Connect(entityData.entity->OnEntityDestruction, [this, uniqueId](Ndk::Entity* /*entity*/)
		{
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
		const std::string& resourceFolder = m_app.GetConfig().GetStringOption("Assets.ResourceFolder");

		std::shared_ptr<VirtualDirectory> assetDir = std::make_shared<VirtualDirectory>(resourceFolder);

		if (!m_assetStore)
			m_assetStore.emplace(GetLogger(), std::move(assetDir));
		else
		{
			m_assetStore->UpdateAssetDirectory(std::move(assetDir));
			m_assetStore->Clear();
		}

		assert(m_map.IsValid());
		for (const auto& asset : m_map.GetAssets())
		{
			Nz::ByteArray checksum(asset.sha1Checksum.size(), 0);
			std::memcpy(checksum.GetBuffer(), asset.sha1Checksum.data(), asset.sha1Checksum.size());

			RegisterAsset(asset.filepath, asset.size, std::move(checksum));
		}
	}

	void Match::ReloadScripts()
	{
		const std::string& scriptFolder = m_app.GetConfig().GetStringOption("Assets.ScriptFolder");

		std::shared_ptr<VirtualDirectory> scriptDir = std::make_shared<VirtualDirectory>(scriptFolder);

		m_clientScripts.clear();

		if (!m_scriptingContext)
		{
			m_scriptingContext = std::make_shared<ScriptingContext>(GetLogger(), scriptDir);
			m_scriptingContext->LoadLibrary(m_scriptingLibrary);
		}
		else
		{
			m_scriptingContext->UpdateScriptDirectory(scriptDir);
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

		VirtualDirectory::Entry entry;

		if (scriptDir->GetEntry("entities", &entry))
		{
			std::filesystem::path path = "entities";

			VirtualDirectory::VirtualDirectoryEntry& directory = std::get<VirtualDirectory::VirtualDirectoryEntry>(entry);
			directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
			{
				m_entityStore->LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), path / entryName);
			});
		}

		if (scriptDir->GetEntry("weapons", &entry))
		{
			std::filesystem::path path = "weapons";

			VirtualDirectory::VirtualDirectoryEntry& directory = std::get<VirtualDirectory::VirtualDirectoryEntry>(entry);
			directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
			{
				m_weaponStore->LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), path / entryName);
			});
		}

		if (!m_gamemode)
			m_gamemode = std::make_shared<ServerGamemode>(*this, m_scriptingContext, m_gamemodePath);
		else
			m_gamemode->Reload();

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

	const Ndk::EntityHandle& Match::RetrieveEntityByUniqueId(Nz::Int64 uniqueId) const
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		if (it == m_entitiesByUniqueId.end())
			return Ndk::EntityHandle::InvalidHandle;

		return it.value().entity;
	}

	Nz::Int64 Match::RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const
	{
		if (!entity || !entity->HasComponent<MatchComponent>())
			return NoEntity;

		return entity->GetComponent<MatchComponent>().GetUniqueId();
	}

	void Match::Update(float elapsedTime)
	{
		m_sessions.Poll();
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
						entityPosition = Nz::Vector2f(entityNode.GetPosition());
						entityRotation = AngleFromQuaternion(entityNode.GetRotation());
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

		m_matchData.gamemodePath = m_gamemodePath.generic_string();
		m_matchData.tickDuration = GetTickDuration();

		m_matchData.layers.clear();
		m_matchData.layers.reserve(mapData.GetLayerCount());
		for (std::size_t i = 0; i < mapData.GetLayerCount(); ++i)
		{
			const auto& mapLayer = mapData.GetLayer(i);

			auto& packetLayer = m_matchData.layers.emplace_back();
			packetLayer.backgroundColor = mapLayer.backgroundColor;
		}

		m_matchData.assets.clear();
		m_matchData.fastDownloadUrls.clear();
		BuildClientAssetListPacket(m_matchData);

		m_matchData.scripts.clear();
		BuildClientScriptListPacket(m_matchData);
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

		m_gamemode->ExecuteCallback("OnPlayerJoin", newPlayer->CreateHandle());

		Packets::ChatMessage chatPacket;
		chatPacket.content = newPlayer->GetName() + " has joined.";

		ForEachPlayer([&](Player* player)
		{
			// Send a PlayerJoined packet to the new player, with everyone
			Packets::PlayerJoined joinedPacket;
			joinedPacket.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
			joinedPacket.playerName = player->GetName();

			newPlayer->SendPacket(joinedPacket);

			chatPacket.localIndex = player->GetLocalIndex();

			player->SendPacket(chatPacket);
		});
	}

	void Match::OnTick(bool lastTick)
	{
		float elapsedTime = GetTickDuration();

		ForEachPlayer([&](Player* player)
		{
			player->OnTick(lastTick);
		});

		m_gamemode->ExecuteCallback("OnTick");

		m_terrain->Update(elapsedTime);

		if (lastTick)
		{
			m_sessions.ForEachSession([&](MatchClientSession* session)
			{
				session->Update(elapsedTime);
			});
		}
	}
	
	void Match::SendPingUpdate()
	{
		Packets::PlayerPingUpdate pingUpdate;

		ForEachPlayer([&](Player* player)
		{
			if (player->IsReady())
			{
				auto& playerData = pingUpdate.players.emplace_back();
				playerData.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
				playerData.ping = static_cast<Nz::UInt16>(player->GetSession().GetPing());
			}
		});

		BroadcastPacket(pingUpdate);
	}
}
