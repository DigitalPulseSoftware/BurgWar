// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientSession.hpp>
#include <Shared/NetworkClientBridge.hpp>
#include <Client/ServerCommandStore.hpp>
#include <Client/BurgApp.hpp>
#include <Nazara/Network/Algorithm.hpp>

namespace bw
{
	ClientSession::~ClientSession() = default;

	bool ClientSession::Connect(const Nz::IpAddress& address)
	{
		Disconnect();

		auto bridge = m_application.ConnectNewServer(address, 0);
		if (!bridge)
			return false;

		//bridge->OnConnected.Connect

		m_bridge = bridge;
		return true;
	}

	bool ClientSession::Connect(const Nz::String& serverHostname, Nz::UInt16 port, Nz::NetProtocol protocol)
	{
		Nz::ResolveError resolveError = Nz::ResolveError_NoError;
		std::vector<Nz::HostnameInfo> results = Nz::IpAddress::ResolveHostname(protocol, serverHostname, Nz::String::Number(port), &resolveError);
		if (results.empty())
		{
			std::cerr << "Failed to resolve server hostname: " << Nz::ErrorToString(resolveError) << std::endl;
			return false;
		}

		return Connect(results.front().address);
	}

	void ClientSession::Disconnect()
	{
		if (m_bridge)
			m_bridge.reset();
	}

	Nz::UInt64 ClientSession::EstimateMatchTime() const
	{
		return m_application.GetAppTime() + m_deltaTime;
	}

	void ClientSession::HandleIncomingPacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(this, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(const Packets::HelloWorld& packet)
	{
		std::cout << "[Client] Hello world: " << packet.str << std::endl;
	}
}
