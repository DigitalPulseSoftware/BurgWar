// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTSESSION_HPP
#define BURGWAR_SERVER_CLIENTSESSION_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/SessionBridge.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Utility/CircularBuffer.hpp>
#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <filesystem>
#include <memory>
#include <vector>

namespace bw
{
	class Match;
	class MatchClientVisibility;
	class Player;
	class PlayerCommandStore;

	using MatchClientSessionHandle = Nz::ObjectHandle<MatchClientSession>;
	using PlayerHandle = Nz::ObjectHandle<Player>;

	// Match client session
	class BURGWAR_CORELIB_API MatchClientSession final : public Nz::HandledObject<MatchClientSession>
	{
		friend PlayerCommandStore;

		public:
			MatchClientSession(Match& match, PlayerCommandStore& commandStore, std::shared_ptr<SessionBridge> bridge);
			MatchClientSession(const MatchClientSession&) = delete;
			MatchClientSession(MatchClientSession&&) = delete;
			~MatchClientSession();

			void Disconnect();

			template<typename F> void ForEachPlayer(F&& func);

			inline Nz::UInt16 GetLastInputTick() const;
			inline Nz::UInt32 GetPing() const;
			inline const SessionBridge& GetSessionBridge() const;
			inline MatchClientVisibility& GetVisibility();
			inline const MatchClientVisibility& GetVisibility() const;

			void HandleIncomingPacket(Nz::NetPacket& packet);

			void OnTick(float elapsedTime);

			template<typename T> void SendPacket(const T& packet);

			void Update(float elapsedTime);

			MatchClientSession& operator=(const MatchClientSession&) = delete;
			MatchClientSession& operator=(MatchClientSession&&) = delete;

		private:
			void HandleIncomingPacket(const Packets::Auth& packet);
			void HandleIncomingPacket(const Packets::DownloadClientFileRequest& packet);
			void HandleIncomingPacket(Packets::PlayerChat&& packet);
			void HandleIncomingPacket(const Packets::PlayerConsoleCommand& packet);
			void HandleIncomingPacket(const Packets::PlayersInput& packet);
			void HandleIncomingPacket(const Packets::PlayerSelectWeapon& packet);
			void HandleIncomingPacket(const Packets::Ready& packet);
			void HandleIncomingPacket(const Packets::ScriptPacket& packet);
			void HandleIncomingPacket(Packets::UpdatePlayerName&& packet);
			void SendClientFile(const std::filesystem::path& filePath);
			void SendClientFile(const std::vector<Nz::UInt8>& content);
			void UpdatePeerInfo(const SessionBridge::SessionInfo& sessionInfo);

			struct Input
			{
				std::vector<std::optional<PlayerInputData>> inputs;
				Nz::UInt16 inputTick;
			};

			/*struct PendingAssetRequest
			{
				std::size_t fragmentCount;
				std::size_t currentFragmentIndex;
				std::filesystem::path filePath;
				Nz::UInt64 fragmentSize;
			};*/

			CircularBuffer<Input> m_queuedInputs;
			Match& m_match;
			PlayerCommandStore& m_commandStore;
			std::shared_ptr<SessionBridge> m_bridge;
			std::unique_ptr<MatchClientVisibility> m_visibility;
			//std::vector<PendingAssetRequest> m_pendingAssetRequest;
			std::vector<PlayerHandle> m_players;
			Nz::UInt16 m_lastInputTick;
			Nz::UInt32 m_ping;
			float m_peerInfoUpdateCounter;
	};
}

#include <CoreLib/MatchClientSession.inl>

#endif
