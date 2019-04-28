// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Match.hpp>
#include <CoreLib/Map.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <Nazara/Core/File.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <cassert>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, const std::string& gamemodeFolder, std::size_t maxPlayerCount, float tickDuration) :
	SharedMatch(app, tickDuration),
	m_gamemodePath(std::filesystem::path("gamemodes") / gamemodeFolder),
	m_sessions(*this),
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName)),
	m_app(app)
	{
		const std::string& scriptFolder = m_app.GetConfig().GetStringOption("Assets.ScriptFolder");

		std::shared_ptr<VirtualDirectory> scriptDir = std::make_shared<VirtualDirectory>(scriptFolder);

		m_scriptingContext = std::make_shared<ScriptingContext>(scriptDir);
		m_scriptingContext->LoadLibrary(std::make_shared<ServerScriptingLibrary>(*this));

		m_gamemode = std::make_shared<ServerGamemode>(*this, m_scriptingContext, m_gamemodePath);

		VirtualDirectory::Entry entry;

		m_entityStore.emplace(m_scriptingContext);

		if (scriptDir->GetEntry("entities", &entry))
			m_entityStore->Load("entities", std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));

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

		m_weaponStore.emplace(app, m_scriptingContext);

		if (scriptDir->GetEntry("weapons", &entry))
			m_weaponStore->Load("weapons", std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));

		m_weaponStore->ForEachElement([&](const ScriptedWeapon& weapon)
		{
			m_networkStringStore.RegisterString(weapon.fullName);

			for (auto&& [propertyName, propertyData] : weapon.properties)
			{
				if (propertyData.shared)
					m_networkStringStore.RegisterString(propertyName);
			}
		});

		Map map = Map::LoadFromBinary("mapdetest.bmap");
		m_terrain = std::make_unique<Terrain>(app, *this, std::move(map));

		m_gamemode->ExecuteCallback("OnInit");
	}

	Match::~Match() = default;

	Packets::ClientScriptList Match::BuildClientFileListPacket() const
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
			for (const Ndk::EntityHandle& entity : layer.GetWorld().GetEntities())
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
	}

	bool Match::GetClientScript(const std::string& filePath, const ClientScript** clientScriptData)
	{
		auto it = m_clientScripts.find(filePath);
		if (it == m_clientScripts.end())
			return false;

		*clientScriptData = &it->second;
		return true;
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

		return true;
	}

	void Match::RegisterClientScript(const std::filesystem::path& clientScript)
	{
		std::string relativePath = clientScript.generic_u8string();

		if (m_clientScripts.find(relativePath) != m_clientScripts.end())
			return;

		const std::string& scriptFolder = m_app.GetConfig().GetStringOption("Assets.ScriptFolder");

		std::string filePath = scriptFolder + "/" + clientScript.generic_u8string();
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

	void Match::Update(float elapsedTime)
	{
		m_sessions.Poll();
		m_scriptingContext->Update();

		SharedMatch::Update(elapsedTime);
	}

	void Match::OnTick()
	{
		float elapsedTime = GetTickDuration();

		m_terrain->Update(elapsedTime);

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			session->Update(elapsedTime);
		});

		m_gamemode->ExecuteCallback("OnTick");
	}
}
