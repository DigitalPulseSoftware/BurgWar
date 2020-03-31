// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/NetworkReactor.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <cassert>

namespace bw
{
	MatchClientSession::MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore, std::shared_ptr<SessionBridge> bridge) :
	m_match(match),
	m_commandStore(commandStore),
	m_sessionId(sessionId),
	m_bridge(std::move(bridge)),
	m_ping(0),
	m_peerInfoUpdateCounter(0.f)
	{
		m_visibility = std::make_unique<MatchClientVisibility>(match, *this);
		m_bridge->OnIncomingPacket.Connect([this](Nz::NetPacket& packet)
		{
			HandleIncomingPacket(packet);
		});
	}

	MatchClientSession::~MatchClientSession()
	{
		ForEachPlayer([this](Player* player)
		{
			m_match.RemovePlayer(player, DisconnectionReason::PlayerLeft);
		});
	}

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

		m_peerInfoUpdateCounter += elapsedTime;
		if (m_peerInfoUpdateCounter >= 1.f)
		{
			m_peerInfoUpdateCounter = 0.f;

			m_bridge->QueryInfo([clientSession = CreateHandle()](const SessionBridge::SessionInfo& info)
			{
				if (!clientSession)
					return; // Session has been destroyed

				clientSession->UpdatePeerInfo(info);
			});
		}
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Auth& packet)
	{
		std::size_t playerCount = packet.players.size();

		bwLog(m_match.GetLogger(), LogLevel::Info, "Auth request for {0} players", playerCount);

		if (playerCount == 0 || playerCount >= 8) //< For now, we don't have any spectator
		{
			SendPacket(Packets::AuthFailure());
			Disconnect();
			return;
		}

		Packets::AuthSuccess authSuccessPacket;

		std::vector<PlayerHandle> players;
		for (std::size_t i = 0; i < packet.players.size(); ++i)
		{
			Player* player = m_match.CreatePlayer(*this, static_cast<Nz::UInt8>(i), packet.players[i].nickname);
			if (!player)
			{
				// FIXME
				for (Player* previousPlayerPleaseFixMe : players)
					m_match.RemovePlayer(previousPlayerPleaseFixMe, DisconnectionReason::Kicked);

				SendPacket(Packets::AuthFailure());
				Disconnect();
				return;
			}

			players.emplace_back(player);
			
			auto& packetPlayer = authSuccessPacket.players.emplace_back();
			packetPlayer.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
		}

		m_players = std::move(players);

		SendPacket(authSuccessPacket);
		SendPacket(m_match.GetNetworkStringStore().BuildPacket());

		SendPacket(m_match.GetMatchData());
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::DownloadClientScriptRequest& packet)
	{
		bwLog(m_match.GetLogger(), LogLevel::Info, "Client asked for client script {0}", packet.path);

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

	void MatchClientSession::HandleIncomingPacket(Packets::PlayerChat&& packet)
	{
		if (packet.localIndex >= m_players.size())
			return;

		if (auto contentOpt = m_match.GetGamemode()->ExecuteCallback("OnPlayerChat", m_players[packet.localIndex]->CreateHandle(), packet.message))
		{
			sol::object& content = *contentOpt;
			if (content.is<sol::nil_t>())
				return;

			if (!content.is<std::string>())
			{
				bwLog(m_match.GetLogger(), LogLevel::Error, "OnPlayerChat was excepted to return a string, but returned a {}", content.get_type());
				return;
			}

			Packets::ChatMessage chatPacket;
			chatPacket.playerIndex = m_players[packet.localIndex]->GetPlayerIndex();
			chatPacket.content = content.as<std::string>();

			m_match.ForEachPlayer([&](Player* player)
			{
				chatPacket.localIndex = player->GetLocalIndex();

				player->SendPacket(chatPacket);
			});
		}
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayerConsoleCommand& packet)
	{
		if (packet.localIndex >= m_players.size())
			return;

		m_players[packet.localIndex]->HandleConsoleCommand(packet.command);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayersInput& packet)
	{
		if (packet.inputs.size() != m_players.size())
		{
			bwLog(m_match.GetLogger(), LogLevel::Error, "Player input count ({0}) doesn't match player count {1}", packet.inputs.size(), m_players.size());
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
		if (packet.localIndex >= m_players.size())
			return;

		Player* player = m_players[packet.localIndex];

		if (packet.newWeaponIndex != packet.NoWeapon && packet.newWeaponIndex >= player->GetWeaponCount())
			return;

		player->SelectWeapon((packet.newWeaponIndex != packet.NoWeapon) ? packet.newWeaponIndex : Player::NoWeapon);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::Ready& /*packet*/)
	{
		ForEachPlayer([this](Player* player)
		{
			m_match.OnPlayerReady(player);
		});
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::ScriptPacket& packet)
	{
		const ScriptHandlerRegistry& registry = m_match.GetScriptPacketHandlerRegistry();
		const NetworkStringStore& stringStore = m_match.GetNetworkStringStore();

		const std::string& packetName = stringStore.GetString(packet.nameIndex);

		registry.Call(packetName, IncomingNetworkPacket(stringStore, packet));
	}

	void MatchClientSession::HandleIncomingPacket(Packets::UpdatePlayerName&& packet)
	{
		if (packet.newName.empty() || packet.newName.size() > 20)
			return;

		if (packet.localIndex >= m_players.size() || !m_players[packet.localIndex])
			return;

		m_players[packet.localIndex]->UpdateName(std::move(packet.newName));
	}
	
	void MatchClientSession::UpdatePeerInfo(const SessionBridge::SessionInfo& sessionInfo)
	{
		m_ping = sessionInfo.ping;
	}
}
