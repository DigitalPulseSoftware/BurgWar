// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/NetworkReactor.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <cassert>

namespace
{
	constexpr Nz::UInt64 MaxFragmentSize = 1200;
}

namespace bw
{
	MatchClientSession::MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore, std::shared_ptr<SessionBridge> bridge) :
	m_queuedInputs(4),
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

	void MatchClientSession::OnTick(float /*elapsedTime*/)
	{
		if (!m_queuedInputs.IsEmpty())
		{
			Input inputData = m_queuedInputs.Dequeue();
			m_lastInputTick = inputData.inputTick;

			for (std::size_t playerIndex = 0; playerIndex < inputData.inputs.size(); ++playerIndex)
			{
				const auto& inputOpt = inputData.inputs[playerIndex];
				if (!inputOpt.has_value())
					continue;

				m_players[playerIndex]->UpdateInputs(*inputOpt);
			}
		}
		/*else
			bwLog(m_match.GetLogger(), LogLevel::Warning, "Player session #{} has no input for this tick", m_sessionId);*/
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

			// If local bridge (local player), set admin by default
			if (m_bridge->IsLocal())
				player->SetAdmin(true);

			players.emplace_back(player);
			
			auto& packetPlayer = authSuccessPacket.players.emplace_back();
			packetPlayer.playerIndex = static_cast<Nz::UInt16>(player->GetPlayerIndex());
		}

		m_players = std::move(players);

		SendPacket(authSuccessPacket);
		SendPacket(m_match.GetNetworkStringStore().BuildPacket());

		SendPacket(m_match.GetMatchData());
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::DownloadClientFileRequest& packet)
	{
		bwLog(m_match.GetLogger(), LogLevel::Info, "Client requested client asset {0}", packet.path);

		const Match::ClientAsset* clientAsset;
		const Match::ClientScript* clientScript;
		if (m_match.GetClientAsset(packet.path, &clientAsset))
			SendClientFile(clientAsset->realPath);
		else if (m_match.GetClientScript(packet.path, &clientScript))
			SendClientFile(clientScript->content);
		else
			Disconnect();
	}

	void MatchClientSession::HandleIncomingPacket(Packets::PlayerChat&& packet)
	{
		if (packet.localIndex >= m_players.size())
			return;

		Player* player = m_players[packet.localIndex];

		auto chatRetOpt = m_match.GetGamemode()->ExecuteCallback<GamemodeEvent::PlayerChat>(m_players[packet.localIndex]->CreateHandle(), packet.message);
		if (!chatRetOpt)
			return m_match.BroadcastChatMessage(player, packet.message);

		GamemodePlayerChatReturn& chatRet = *chatRetOpt;
		if (chatRet.first.has_value() && !*chatRet.first)
			return;

		if (chatRet.second)
			m_match.BroadcastChatMessage(player, std::move(*chatRet.second));
		else
			m_match.BroadcastChatMessage(player, packet.message);
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

		m_queuedInputs.Enqueue(Input{ std::move(packet.inputs), packet.inputTick });
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::PlayerSelectWeapon& packet)
	{
		if (packet.localIndex >= m_players.size())
			return;

		Player* player = m_players[packet.localIndex];

		const Ndk::EntityHandle& controlledEntity = player->GetControlledEntity();
		if (!controlledEntity)
			return;

		auto& entityWeapons = controlledEntity->GetComponent<WeaponWielderComponent>();

		if (packet.newWeaponIndex >= entityWeapons.GetWeaponCount() && packet.newWeaponIndex != packet.NoWeapon)
			return;

		entityWeapons.SelectWeapon((packet.newWeaponIndex != packet.NoWeapon) ? packet.newWeaponIndex : Player::NoWeapon);
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

	void MatchClientSession::SendClientFile(const std::filesystem::path& filePath)
	{
		if (!std::filesystem::is_regular_file(filePath))
		{
			bwLog(m_match.GetLogger(), LogLevel::Error, "Client asset {} does not exist", filePath.generic_u8string());

			Packets::DownloadClientFileResponse response;
			auto& failure = response.content.emplace<Packets::DownloadClientFileResponse::Failure>();
			failure.error = Packets::DownloadClientFileResponse::Error::FileNotFound;

			SendPacket(response);
			return;
		}

		//FIXME: Use fragments instead of sending the whole file at once
		Nz::File file(filePath.generic_u8string(), Nz::OpenMode_ReadOnly);
		if (!file.IsOpen())
		{
			bwLog(m_match.GetLogger(), LogLevel::Error, "Failed to open {}", filePath.generic_u8string());

			// An error occurred, send 0 fragment to notify the issue
			Packets::DownloadClientFileResponse response;
			auto& failure = response.content.emplace<Packets::DownloadClientFileResponse::Failure>();
			failure.error = Packets::DownloadClientFileResponse::Error::FileNotFound;

			SendPacket(response);
			return;
		}

		std::vector<Nz::UInt8> content(file.GetSize());
		if (file.Read(content.data(), content.size()) != content.size())
		{
			bwLog(m_match.GetLogger(), LogLevel::Error, "Failed to read {}", filePath.generic_u8string());

			// An error occurred, send 0 fragment to notify the issue
			Packets::DownloadClientFileResponse response;
			auto& failure = response.content.emplace<Packets::DownloadClientFileResponse::Failure>();
			failure.error = Packets::DownloadClientFileResponse::Error::FileNotFound;

			SendPacket(response);
			return;
		}

		file.Close();

		bwLog(m_match.GetLogger(), LogLevel::Info, "Sending asset {}", filePath.generic_u8string());
		SendClientFile(content);
	}

	void MatchClientSession::SendClientFile(const std::vector<Nz::UInt8>& content)
	{
		//std::size_t fragmentCount = clientAsset->size / MaxFragmentSize + ((clientAsset->size % MaxFragmentSize != 0) ? 1 : 0);

		/*auto& pendingRequest = m_pendingAssetRequest.emplace_back();
		pendingRequest.currentFragmentIndex = 0;
		pendingRequest.filePath = clientAsset->path;
		pendingRequest.fragmentCount = fragmentCount;
		pendingRequest.fragmentSize = MaxFragmentSize;*/

		Packets::DownloadClientFileResponse response;
		auto& success = response.content.emplace<Packets::DownloadClientFileResponse::Success>();
		//success.fragmentCount = static_cast<Nz::UInt32>(fragmentCount);
		success.fragmentCount = 1;
		success.fragmentSize = content.size();

		SendPacket(response);

		Packets::DownloadClientFileFragment fragment;
		fragment.fragmentIndex = 0;
		fragment.fragmentContent = std::move(content);

		SendPacket(fragment);
	}
	
	void MatchClientSession::UpdatePeerInfo(const SessionBridge::SessionInfo& sessionInfo)
	{
		m_ping = sessionInfo.ping;
	}
}
