// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientSession.hpp>
#include <Shared/Match.hpp>
#include <Shared/MapData.hpp>
#include <Shared/NetworkReactor.hpp>
#include <Shared/PlayerCommandStore.hpp>
#include <Shared/Terrain.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	void MatchClientSession::Disconnect()
	{
		m_bridge->Disconnect();
	}

	void MatchClientSession::HandleIncomingPacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(*this, std::move(packet));
	}

	void MatchClientSession::Update(float elapsedTime)
	{
		m_visibility.Update(elapsedTime);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Auth& packet)
	{
		std::cout << "[Server] Auth request for " << packet.playerCount << " players" << std::endl;

		if (packet.playerCount == 0 || packet.playerCount >= 8) //< For now, we don't have any spectator
		{
			SendPacket(Packets::AuthFailure());
			Disconnect();
			return;
		}

		assert(packet.playerCount != 0xFF);

		m_players.reserve(packet.playerCount);
		for (Nz::UInt8 i = 0; i < packet.playerCount; ++i)
		{
			Player& player = m_players.emplace_back(*this, "Noname");
			if (!m_match.Join(&player))
			{
				m_players.clear();
				SendPacket(Packets::AuthFailure());
				Disconnect();
				return;
			}
		}

		SendPacket(Packets::AuthSuccess());

		const MapData& mapData = m_match.GetTerrain().GetMapData();

		Packets::MatchData matchData;
		matchData.backgroundColor = mapData.backgroundColor;
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

			m_players[playerIndex].UpdateInput(input->isJumping, input->isMovingLeft, input->isMovingRight);
		}
	}
}
