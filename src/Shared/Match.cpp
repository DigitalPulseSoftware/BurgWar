// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Match.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/Player.hpp>
#include <Shared/Terrain.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <cassert>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, std::size_t maxPlayerCount) :
	m_sessions(*this),
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName)),
	m_application(app)
	{
		MapData mapData;
		mapData.backgroundColor = Nz::Color::Cyan;
		mapData.tileSize = 64.f;

		auto& layer = mapData.layers.emplace_back();
		layer.width = 20;
		layer.height = 10;
		layer.tiles = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 1, 0, 2, 2,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1,
		};

		m_terrain = std::make_unique<Terrain>(std::move(mapData));
	}

	Match::~Match() = default;

	void Match::Leave(Player* player)
	{
		assert(player->GetMatch() == this);

		auto it = std::find(m_players.begin(), m_players.end(), player);
		assert(it != m_players.end());

		m_players.erase(it);

		player->UpdateLayer(std::numeric_limits<std::size_t>::max());
		player->UpdateMatch(nullptr);
	}

	bool Match::Join(Player* player)
	{
		assert(!player->IsInMatch());

		if (m_players.size() >= m_maxPlayerCount)
			return false;

		m_players.push_back(player);
		player->UpdateMatch(this);

		player->UpdateLayer(0);
		player->CreateEntity(m_terrain->GetLayer(0).GetWorld());

		return true;
	}

	void Match::Update(float elapsedTime)
	{
		m_sessions.Poll();
		m_terrain->Update(elapsedTime);

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			Packets::CreateEntities createPacket;
			Packets::DeleteEntities deletePacket;
			Packets::MatchState statePacket;

			for (std::size_t i = 0; i < m_terrain->GetLayerCount(); ++i)
			{
				TerrainLayer& layer = m_terrain->GetLayer(i);
				auto& networkSync = layer.GetWorld().GetSystem<NetworkSyncSystem>();
				for (const auto& event : networkSync.GetEvents())
				{
					std::visit([&](auto&& eventData)
					{
						using T = std::decay_t<decltype(eventData)>;

						if constexpr (std::is_same_v<T, NetworkSyncSystem::Event::EntityCreation>)
						{
							auto& entityData = createPacket.entities.emplace_back();
							entityData.id = event.id;
							entityData.angularVelocity = eventData.angularVelocity;
							entityData.linearVelocity = eventData.linearVelocity;
							entityData.position = eventData.position;
							entityData.rotation = eventData.rotation;
							entityData.hasPlayerMovement = eventData.hasPlayerMovement;
						}
						else if constexpr (std::is_same_v<T, NetworkSyncSystem::Event::EntityDestruction>)
						{
							auto& entityData = deletePacket.entityIds.emplace_back();
							entityData.id = event.id;
						}
						else if constexpr (std::is_same_v<T, NetworkSyncSystem::Event::EntityMovement>)
						{
							auto& entityData = statePacket.entities.emplace_back();
							entityData.id = event.id;
							entityData.angularVelocity = eventData.angularVelocity;
							entityData.linearVelocity = eventData.linearVelocity;
							entityData.position = eventData.position;
							entityData.rotation = eventData.rotation;

							if (eventData.playerMovement.has_value())
							{
								entityData.playerMovement.emplace();
								entityData.playerMovement->isAirControlling = eventData.playerMovement->isAirControlling;
								entityData.playerMovement->isFacingRight = eventData.playerMovement->isFacingRight;
							}
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

					}, event.eventData);
				}
			}

			if (!deletePacket.entityIds.empty())
				session->SendPacket(deletePacket);

			if (!createPacket.entities.empty())
				session->SendPacket(createPacket);

			if (!statePacket.entities.empty())
				session->SendPacket(statePacket);
		});

		for (std::size_t i = 0; i < m_terrain->GetLayerCount(); ++i)
		{
			TerrainLayer& layer = m_terrain->GetLayer(i);
			auto& networkSync = layer.GetWorld().GetSystem<NetworkSyncSystem>();
			networkSync.ClearEvents();
		}
	}
}
