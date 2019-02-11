// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Match.hpp>
#include <GameLibShared/MatchClientSession.hpp>
#include <GameLibShared/Player.hpp>
#include <GameLibShared/Terrain.hpp>
#include <GameLibShared/Scripting/ServerGamemode.hpp>
#include <GameLibShared/Protocol/Packets.hpp>
#include <GameLibShared/Systems/NetworkSyncSystem.hpp>
#include <Nazara/Core/File.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <cassert>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, const std::string& gamemodeFolder, std::size_t maxPlayerCount) :
	SharedMatch(app),
	m_gamemodePath(std::filesystem::path("gamemodes") / gamemodeFolder),
	m_sessions(*this),
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName))
	{
		MapData mapData;
		mapData.backgroundColor = Nz::Color::Cyan;
		mapData.tileSize = 64.f;

		auto& layer = mapData.layers.emplace_back();
		layer.width = 80;
		layer.height = 13;
		/*layer.tiles = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
		};*/

		for (std::size_t y = 0; y < layer.height; ++y)
		{
			Nz::UInt8 blockType = 0;
			if (y == 10)
				blockType = 2;
			else if (y > 10)
				blockType = 1;

			Nz::UInt8 originalBlockType = blockType;
			for (std::size_t x = 0; x < layer.width; ++x)
			{
				if (x == 0 || x == layer.width - 1)
					blockType = 1;
				else if (x > 10 && x < 15)
					blockType = 0;
				else
					blockType = originalBlockType;

				layer.tiles.push_back(blockType);
			}
		}

		m_terrain = std::make_unique<Terrain>(app, std::move(mapData));

		m_scriptingContext = std::make_shared<ServerScriptingContext>(*this);

		m_gamemode = std::make_shared<ServerGamemode>(*this, m_scriptingContext, "../../scripts" / m_gamemodePath);

		m_entityStore.emplace(m_gamemode, m_scriptingContext);
		m_entityStore->Load("../../scripts/entities");

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

		m_weaponStore.emplace(app, m_gamemode, m_scriptingContext);
		m_weaponStore->Load("../../scripts/weapons");

		m_weaponStore->ForEachElement([&](const ScriptedWeapon& weapon)
		{
			m_networkStringStore.RegisterString(weapon.fullName);

			for (auto&& [propertyName, propertyData] : weapon.properties)
			{
				if (propertyData.shared)
					m_networkStringStore.RegisterString(propertyName);
			}
		});

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
		player->CreateEntity(m_terrain->GetLayer(0).GetWorld());

		return true;
	}

	void Match::RegisterClientScript(const std::filesystem::path& clientScript)
	{
		std::filesystem::path scriptPath = "../../scripts";
		std::string relativePath = std::filesystem::relative(clientScript, scriptPath).generic_u8string();

		if (m_clientScripts.find(relativePath) != m_clientScripts.end())
			return;

		std::string filePath = clientScript.generic_u8string();
		if (!std::filesystem::is_regular_file(clientScript))
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
		SharedMatch::Update(elapsedTime);

		m_scriptingContext->Update();
		m_sessions.Poll();
		m_terrain->Update(elapsedTime);

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			session->Update(elapsedTime);
		});

		m_gamemode->ExecuteCallback("OnTick");
	}
}
