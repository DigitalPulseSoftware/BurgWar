// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientSession.hpp>
#include <Shared/NetworkClientBridge.hpp>
#include <Client/BurgApp.hpp>
#include <Client/LocalCommandStore.hpp>
#include <Client/LocalMatch.hpp>
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

		bridge->OnConnected.Connect([this](Nz::UInt32 /*data*/)
		{
			m_isConnected = true;
		});

		bridge->OnDisconnected.Connect([this](Nz::UInt32 /*data*/)
		{
			m_isConnected = false;
			m_bridge.reset();
		});

		bridge->OnIncomingPacket.Connect([this](Nz::NetPacket& packet)
		{
			HandleIncomingPacket(packet);
		});

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
		{
			m_bridge->Disconnect();
			m_bridge.reset();
		}
	}

	Nz::UInt64 ClientSession::EstimateMatchTime() const
	{
		return m_application.GetAppTime() + m_deltaTime;
	}

	void ClientSession::HandleIncomingPacket(Nz::NetPacket& packet)
	{
		m_commandStore.UnserializePacket(this, packet);
	}

	void ClientSession::HandleIncomingPacket(const Packets::AuthFailure& packet)
	{
		std::cout << "[Client] Auth failed" << std::endl;
	}

	void ClientSession::HandleIncomingPacket(const Packets::AuthSuccess& packet)
	{
		std::cout << "[Client] Auth succeeded" << std::endl;
	}

	void ClientSession::HandleIncomingPacket(const Packets::CreateEntities& packet)
	{
		for (const auto& entityData : packet.entities)
		{
			std::cout << "[Client] Entity #" << entityData.id << " created at " << entityData.position << std::endl;

			m_localMatch->CreateEntity(entityData.id, entityData.position, entityData.playerMovement.has_value());
		}
	}

	void ClientSession::HandleIncomingPacket(const Packets::DeleteEntities& packet)
	{
		for (const auto& entityData : packet.entities)
		{
			std::cout << "[Client] Entity #" << entityData.id << " deleted" << std::endl;
			m_localMatch->DeleteEntity(entityData.id);
		}
	}

	void ClientSession::HandleIncomingPacket(const Packets::HelloWorld& packet)
	{
		std::cout << "[Client] Hello world: " << packet.str << std::endl;
	}

	void ClientSession::HandleIncomingPacket(const Packets::MatchData& matchData)
	{
		std::cout << "[Client] Got match data: " << matchData.backgroundColor << std::endl;
		m_localMatch = m_application.CreateLocalMatch(*this, matchData);
	}

	void ClientSession::HandleIncomingPacket(const Packets::MatchState& packet)
	{
		for (const auto& entityData : packet.entities)
		{
			bool isAirControlling = false;
			bool isFacingRight = false;

			std::cout << "[Client] Entity #" << entityData.id << " is now at " << entityData.position;
			if (entityData.playerMovement.has_value())
			{
				isAirControlling = entityData.playerMovement->isAirControlling;
				isFacingRight = entityData.playerMovement->isFacingRight;
				std::cout << " and has player entity data (air control=" << isAirControlling << ", facing right=" << isFacingRight << ")";
			}

			std::cout << "\n";

			m_localMatch->MoveEntity(entityData.id, entityData.position, entityData.linearVelocity, entityData.rotation, entityData.angularVelocity, isAirControlling, isFacingRight);
		}
	}
}
