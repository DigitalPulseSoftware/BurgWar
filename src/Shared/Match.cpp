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
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName)),
	m_application(app)
	{
		m_terrain = std::make_unique<Terrain>();
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
