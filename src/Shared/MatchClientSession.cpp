// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientSession.hpp>
#include <Shared/PlayerCommandStore.hpp>
#include <Shared/NetworkReactor.hpp>
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

	void MatchClientSession::HandleIncomingPacket(const Packets::Auth& packet)
	{
		std::cout << "[Server] Auth request" << std::endl;

		SendPacket(Packets::AuthSuccess());

		Packets::MatchData matchData;
		matchData.backgroundColor = Nz::Color::Cyan;
		matchData.tileSize = 64.f;
		matchData.width = 15;
		matchData.height = 10;
		matchData.tiles = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
			2, 2, 2, 2, 2, 1, 2, 0, 0, 1, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0,
		};

		SendPacket(matchData);
	}

	void MatchClientSession::HandleIncomingPacket(const Packets::HelloWorld& packet)
	{
		std::cout << "[Server] Hello world: " << packet.str << std::endl;

		Packets::HelloWorld hw;
		hw.str = "La belgique aurait dû gagner la coupe du monde 2018";

		SendPacket(hw);
	}
}
