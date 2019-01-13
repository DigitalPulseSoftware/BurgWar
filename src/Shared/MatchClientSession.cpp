// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientSession.hpp>
#include <Shared/Match.hpp>
#include <Shared/MatchClientVisibility.hpp>
#include <Shared/MapData.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Shared/Player.hpp>
#include <Shared/PlayerCommandStore.hpp>
#include <Shared/Terrain.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	MatchClientSession::MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore, std::shared_ptr<SessionBridge> bridge) :
	m_match(match),
	m_commandStore(commandStore),
	m_sessionId(sessionId),
	m_bridge(std::move(bridge))
	{
		m_visibility = std::make_unique<MatchClientVisibility>(match, *this);
		m_bridge->OnIncomingPacket.Connect([this](Nz::NetPacket& packet)
		{
			HandleIncomingPacket(packet);
		});
	}

	MatchClientSession::~MatchClientSession() = default;

	void MatchClientSession::Disconnect()
	{
		m_bridge->Disconnect();
	}

	void MatchClientSession::HandleIncomingPacket(Nz::NetPacket& packet)
	{
		m_commandStore.UnserializePacket(*this, packet);
	}

	void MatchClientSession::Update(float elapsedTime)
	{
		m_visibility->Update(elapsedTime);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Auth& packet)
	{
		std::cout << "[Server] Auth request for " << +packet.playerCount << " players" << std::endl;

		if (packet.playerCount == 0 || packet.playerCount >= 8) //< For now, we don't have any spectator
		{
			SendPacket(Packets::AuthFailure());
			Disconnect();
			return;
		}

		assert(packet.playerCount != 0xFF);

		std::vector<Player> players;
		for (Nz::UInt8 i = 0; i < packet.playerCount; ++i)
		{
			Player& player = players.emplace_back(*this, i, "Noname");
			if (!m_match.Join(&player))
			{
				SendPacket(Packets::AuthFailure());
				Disconnect();
				return;
			}
		}

		m_players = std::move(players);

		SendPacket(Packets::AuthSuccess());
		SendPacket(m_match.GetNetworkStringStore().BuildPacket());

		// Send match data
		const MapData& mapData = m_match.GetTerrain().GetMapData();

		Packets::MatchData matchData;
		matchData.backgroundColor = mapData.backgroundColor;
		matchData.gamemodePath = m_match.GetGamemodePath().generic_string();
		matchData.tileSize = mapData.tileSize;

		matchData.layers.reserve(mapData.layers.size());
		for (auto& layer : mapData.layers)
		{
			auto& packetLayer = matchData.layers.emplace_back();
			packetLayer.height = static_cast<Nz::UInt16>(layer.height);
			packetLayer.width = static_cast<Nz::UInt16>(layer.width);
			packetLayer.tiles = layer.tiles;
		}

		SendPacket(matchData);

		// Send client-file script list
		SendPacket(m_match.BuildClientFileListPacket());
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::DownloadClientScriptRequest& packet)
	{
		std::cout << "[Server] Client asked for client script " << packet.path << std::endl;

		const Match::ClientScript* clientScript;
		if (m_match.GetClientScript(packet.path, &clientScript))
		{
			Packets::DownloadClientScriptResponse response;
			response.fileContent = clientScript->content;

			SendPacket(response);
		}
		else
			Disconnect();
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::HelloWorld& packet)
	{
		std::cout << "[Server] Hello world: " << packet.str << std::endl;

		Packets::HelloWorld hw;
		hw.str = "La belgique aurait dû gagner la coupe du monde 2018";

		SendPacket(hw);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayersInput& packet)
	{
		if (packet.inputs.size() != m_players.size())
		{
			std::cerr << "Player input count doesn't match player count" << std::endl;
			return;
		}

		for (std::size_t playerIndex = 0; playerIndex < packet.inputs.size(); ++playerIndex)
		{
			const auto& input = packet.inputs[playerIndex];
			if (!input.has_value())
				continue;

			m_players[playerIndex].UpdateInput(input->isAttacking, input->isJumping, input->isMovingLeft, input->isMovingRight);
		}
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Ready& packet)
	{
		// HAAAAAX
		GetVisibility().UpdateLayer(0);
	}
}
