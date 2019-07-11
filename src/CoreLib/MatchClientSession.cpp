// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/NetworkReactor.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
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
		m_visibility->Update();
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Auth& packet)
	{
		std::size_t playerCount = packet.players.size();

		std::cout << "[Server] Auth request for " << playerCount << " players" << std::endl;

		if (playerCount == 0 || playerCount >= 8) //< For now, we don't have any spectator
		{
			SendPacket(Packets::AuthFailure());
			Disconnect();
			return;
		}

		std::vector<std::unique_ptr<Player>> players;
		for (std::size_t i = 0; i < packet.players.size(); ++i)
		{
			std::unique_ptr<Player> player = std::make_unique<Player>(*this, static_cast<Nz::UInt8>(i), packet.players[i].nickname);
			if (!m_match.Join(player.get()))
			{
				SendPacket(Packets::AuthFailure());
				Disconnect();
				return;
			}

			players.emplace_back(std::move(player));
		}

		m_players = std::move(players);

		SendPacket(Packets::AuthSuccess());
		SendPacket(m_match.GetNetworkStringStore().BuildPacket());

		// Send match data
		const Map& mapData = m_match.GetTerrain().GetMap();

		Packets::MatchData matchData;
		matchData.currentTick = m_match.GetNetworkTick();
		matchData.gamemodePath = m_match.GetGamemodePath().generic_string();
		matchData.tickDuration = m_match.GetTickDuration();

		matchData.layers.reserve(mapData.GetLayerCount());
		for (std::size_t i = 0; i < mapData.GetLayerCount(); ++i)
		{
			const auto& mapLayer = mapData.GetLayer(i);

			auto& packetLayer = matchData.layers.emplace_back();
			packetLayer.backgroundColor = mapLayer.backgroundColor;
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

	void MatchClientSession::HandleIncomingPacket(Packets::PlayerChat&& packet)
	{
		if (packet.playerIndex >= m_players.size())
			return;

		Packets::ChatMessage chatPacket;
		chatPacket.content = m_players[packet.playerIndex]->GetName() + ": " + std::move(packet.message);

		//FIXME: Should be for each session
		m_match.ForEachPlayer([&](Player* player)
		{
			player->SendPacket(chatPacket);
		});
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayerConsoleCommand& packet)
	{
		if (packet.playerIndex >= m_players.size())
			return;

		m_players[packet.playerIndex]->HandleConsoleCommand(packet.command);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayersInput& packet)
	{
		if (packet.inputs.size() != m_players.size())
		{
			std::cerr << "Player input count doesn't match player count" << std::endl;
			return;
		}

		// Compute client error
		Nz::UInt16 currentTick = m_match.GetNetworkTick();
		Nz::UInt16 adjustedTick = currentTick + 2; // Prevent network jitter
		Nz::UInt16 estimatedServerTick = packet.estimatedServerTick;

		//std::cout << "[Server] Estimated server tick: " << estimatedServerTick << " (current tick: " << adjustedTick << ")" << std::endl;

		// Prevent overflow/underflow
		Nz::Int32 tickError;
		if (estimatedServerTick >= adjustedTick)
			tickError = static_cast<Nz::Int32>(estimatedServerTick - adjustedTick);
		else
			tickError = -static_cast<Nz::Int32>(adjustedTick - estimatedServerTick);

		Packets::InputTimingCorrection correctionPacket;
		correctionPacket.serverTick = packet.estimatedServerTick;
		correctionPacket.tickError = tickError;

		SendPacket(correctionPacket);

		if (estimatedServerTick < currentTick)
			return; //< Tick has already been simulated, ignore

		if (estimatedServerTick >= currentTick + 10)
			return; //< Tick is way off prediction

		std::size_t tickDelay = estimatedServerTick - currentTick;

		for (std::size_t playerIndex = 0; playerIndex < packet.inputs.size(); ++playerIndex)
		{
			const auto& inputOpt = packet.inputs[playerIndex];
			if (!inputOpt.has_value())
				continue;

			m_players[playerIndex]->UpdateInputs(tickDelay, *inputOpt);
		}
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayerSelectWeapon& packet)
	{
		if (packet.playerIndex >= m_players.size())
			return;

		Player* player = m_players[packet.playerIndex].get();

		if (packet.newWeaponIndex != packet.NoWeapon && packet.newWeaponIndex >= player->GetWeaponCount())
			return;

		player->SelectWeapon((packet.newWeaponIndex != packet.NoWeapon) ? packet.newWeaponIndex : Player::NoWeapon);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Ready& packet)
	{
		// HAAAAAX
		GetVisibility().UpdateLayer(0);
	}
}
