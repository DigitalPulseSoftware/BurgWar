// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Match.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Scripting/ServerElementLibrary.hpp>
#include <CoreLib/Scripting/ServerEntityLibrary.hpp>
#include <CoreLib/Scripting/ServerWeaponLibrary.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Protocol/CompressedInteger.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <Nazara/Core/File.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <cassert>
#include <fstream>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, const std::string& gamemodeFolder, std::size_t maxPlayerCount, float tickDuration) :
	SharedMatch(tickDuration),
	m_gamemodePath(std::filesystem::path("gamemodes") / gamemodeFolder),
	m_sessions(*this),
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName)),
	m_app(app)
	{
		m_scriptingLibrary = std::make_shared<ServerScriptingLibrary>(*this);

		m_map = Map::LoadFromBinary(app.GetConfig().GetStringOption("GameSettings.MapFile"));

		ReloadAssets();
		ReloadScripts();

		m_terrain = std::make_unique<Terrain>(*this, *m_map);

		m_gamemode->ExecuteCallback("OnInit");

		if (m_app.GetConfig().GetBoolOption("Debug.SendServerState"))
		{
			m_debug.emplace();
			if (m_debug->socket.Create(Nz::NetProtocol_IPv4))
				m_debug->socket.EnableBlocking(false);
			else
			{
				std::cerr << "Failed to create debug socket";
				m_debug.reset();
			}
		}
	}

	Match::~Match()
	{
		GetTimerManager().Clear();
	}

	Packets::ClientAssetList Match::BuildClientAssetListPacket() const
	{
		Packets::ClientAssetList clientAsset;
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

		return clientAsset;
	}

	Packets::ClientScriptList Match::BuildClientScriptListPacket() const
	{
		Packets::ClientScriptList clientScript;

		for (const auto& pair : m_clientScripts)
		{
			auto& scriptData = clientScript.scripts.emplace_back();
			scriptData.path = pair.first;

			const Nz::ByteArray& checksum = pair.second.checksum;
			assert(scriptData.sha1Checksum.size() == checksum.size());
			std::memcpy(scriptData.sha1Checksum.data(), checksum.GetConstBuffer(), checksum.GetSize());
		}

		std::sort(clientScript.scripts.begin(), clientScript.scripts.end(), [](const auto& first, const auto& second) { return first.path < second.path; });

		return clientScript;
	}

	void Match::ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func)
	{
		for (std::size_t i = 0; i < m_terrain->GetLayerCount(); ++i)
		{
			auto& layer = m_terrain->GetLayer(i);
			for (const Ndk::EntityHandle& entity : layer.GetWorld().GetWorld().GetEntities())
				func(entity);
		}
	}

	void Match::Leave(Player* player)
	{
		assert(player->GetMatch() == this);

		auto it = std::find(m_players.begin(), m_players.end(), player);
		assert(it != m_players.end());

		m_players.erase(it);

		player->UpdateLayer(std::numeric_limits<std::size_t>::max());
		player->UpdateMatch(nullptr);

		Packets::ChatMessage chatPacket;
		chatPacket.content = player->GetName() + " has left.";

		//FIXME: Should be for each session
		ForEachPlayer([&](Player* player)
		{
			player->SendPacket(chatPacket);
		});
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

	ServerWeaponStore& Match::GetWeaponStore()
	{
		return *m_weaponStore;
	}

	const ServerWeaponStore& Match::GetWeaponStore() const
	{
		return *m_weaponStore;
	}

	bool Match::Join(Player* player)
	{
		assert(!player->IsInMatch());

		if (m_players.size() >= m_maxPlayerCount)
			return false;

		m_players.emplace_back(player);
		player->UpdateMatch(this);

		player->UpdateLayer(0);

		m_gamemode->ExecuteCallback("OnPlayerJoin", player->CreateHandle());

		Packets::ChatMessage chatPacket;
		chatPacket.content = player->GetName() + " has joined.";

		//FIXME: Should be for each session
		ForEachPlayer([&](Player* player)
		{
			player->SendPacket(chatPacket);
		});

		return true;
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
				std::cerr << "Asset " << assetPath << " registered twice and size doesn't match" << std::endl;
				return;
			}

			if (asset.checksum != assetChecksum)
			{
				std::cerr << "Asset " << assetPath << " registered twice and checksum doesn't match" << std::endl;
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

	void Match::ReloadAssets()
	{
		const std::string& resourceFolder = m_app.GetConfig().GetStringOption("Assets.ResourceFolder");

		std::shared_ptr<VirtualDirectory> assetDir = std::make_shared<VirtualDirectory>(resourceFolder);

		if (!m_assetStore)
			m_assetStore.emplace(std::move(assetDir));
		else
		{
			m_assetStore->UpdateAssetDirectory(std::move(assetDir));
			m_assetStore->Clear();
		}

		assert(m_map);
		for (const auto& asset : m_map->GetAssets())
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
			m_scriptingContext = std::make_shared<ScriptingContext>(scriptDir);
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
				serverElementLib = std::make_shared<ServerElementLibrary>();

			m_entityStore.emplace(m_scriptingContext);
			m_entityStore->LoadLibrary(serverElementLib);
			m_entityStore->LoadLibrary(std::make_shared<ServerEntityLibrary>());
		}
		else
		{
			m_entityStore->ClearElements();
			m_entityStore->ReloadLibraries();
		}

		if (!m_weaponStore)
		{
			if (!serverElementLib)
				serverElementLib = std::make_shared<ServerElementLibrary>();

			m_weaponStore.emplace(m_scriptingContext);
			m_weaponStore->LoadLibrary(serverElementLib);
			m_weaponStore->LoadLibrary(std::make_shared<ServerWeaponLibrary>(*this));
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

		Packets::ClientScriptList clientScriptPacket = BuildClientScriptListPacket();
		// FIXME: Should be for each session
		ForEachPlayer([&](Player* player)
		{
			player->SendPacket(clientScriptPacket);
		});
	}

	void Match::Update(float elapsedTime)
	{
		m_sessions.Poll();
		m_scriptingContext->Update();

		SharedMatch::Update(elapsedTime);

		if (m_debug && m_app.GetAppTime() - m_debug->lastBroadcastTime > 1000 / 60)
		{
			m_debug->lastBroadcastTime = m_app.GetAppTime();

			// Send all entities state
			Nz::NetPacket debugPacket(1);

			std::size_t offset = debugPacket.GetStream()->GetCursorPos();

			Nz::UInt32 entityCount = 0;
			debugPacket << entityCount;

			ForEachEntity([&](const Ndk::EntityHandle& entity)
			{
				if (!entity->HasComponent<Ndk::NodeComponent>() || !entity->HasComponent<NetworkSyncComponent>())
					return;

				auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

				entityCount++;

				CompressedUnsigned<Nz::UInt32> entityId(entity->GetId());
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
					entityRotation = Nz::RadianAnglef::FromDegrees(entityNode.GetRotation().ToEulerAngles().roll);
				}

				debugPacket << entityPosition << entityRotation;
			});

			debugPacket.GetStream()->SetCursorPos(offset);
			debugPacket << entityCount;

			Nz::IpAddress localAddress = Nz::IpAddress::LoopbackIpV4;
			for (std::size_t i = 0; i < 4; ++i)
			{
				localAddress.SetPort(static_cast<Nz::UInt16>(42000 + i));

				if (!m_debug->socket.SendPacket(localAddress, debugPacket))
				{
					std::cerr << "Failed to send debug packet: " << Nz::ErrorToString(m_debug->socket.GetLastError()) << std::endl;
				}
			}
		}
	}

	void Match::OnTick(bool lastTick)
	{
		float elapsedTime = GetTickDuration();

		for (Player* player : m_players)
			player->OnTick(lastTick);

		m_gamemode->ExecuteCallback("OnTick");

		m_terrain->Update(elapsedTime);

#ifdef DEBUG_PREDICTION
		ForEachEntity([&](const Ndk::EntityHandle& entity)
		{
			if (entity->HasComponent<InputComponent>() && entity->HasComponent< Ndk::PhysicsComponent2D>())
			{
				auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
				
				static std::ofstream debugFile("server.csv", std::ios::trunc);
				debugFile << m_app.GetAppTime() << ";" << ((entity->GetComponent<InputComponent>().GetInputs().isJumping) ? "Jumping;" : ";") << GetNetworkTick() << ";" << entityPhys.GetPosition().y << ";" << entityPhys.GetVelocity().y << '\n';
			}
		});
#endif

		if (lastTick)
		{
			m_sessions.ForEachSession([&](MatchClientSession* session)
			{
				session->Update(elapsedTime);
			});
		}
	}
}
