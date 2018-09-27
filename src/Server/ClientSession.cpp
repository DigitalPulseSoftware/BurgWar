// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ClientSession.hpp>
#include <Server/ClientCommandStore.hpp>
#include <Shared/NetworkReactor.hpp>
#include <iostream>

namespace bw
{
	void ClientSession::Disconnect()
	{
		m_reactor.DisconnectPeer(m_peerId);
	}

	void ClientSession::HandlePacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(*this, std::move(packet));
	}

	void ClientSession::HandlePacket(const Packets::HelloWorld& packet)
	{
		std::cout << "Client " << m_sessionId << " says hi (" << packet.str << ")" << std::endl;

		Packets::HelloWorld answer;
		answer.str = "La belgique aurait dû gagner la demi-finale 2018";

		SendPacket(answer);
	}
}
