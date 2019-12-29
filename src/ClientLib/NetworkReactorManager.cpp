// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/NetworkReactorManager.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <CoreLib/LogSystem/Logger.hpp>

namespace bw
{
	std::shared_ptr<NetworkSessionBridge> NetworkReactorManager::ConnectToServer(const Nz::IpAddress& serverAddress, Nz::UInt32 data)
	{
		constexpr std::size_t MaxPeerCount = 1;

		auto ConnectWithReactor = [&](NetworkReactor* reactor) -> std::shared_ptr<NetworkSessionBridge>
		{
			std::size_t newPeerId = reactor->ConnectTo(serverAddress, data);
			if (newPeerId == NetworkReactor::InvalidPeerId)
			{
				bwLog(m_logger, LogLevel::Error, "Failed to allocate new peer");
				return nullptr;
			}

			auto bridge = std::make_shared<NetworkSessionBridge>(*reactor, newPeerId);

			if (newPeerId >= m_connections.size())
				m_connections.resize(newPeerId + 1);

			m_connections[newPeerId] = bridge;
			return bridge;
		};

		std::size_t reactorCount = GetReactorCount();
		std::size_t reactorIndex;
		for (reactorIndex = 0; reactorIndex < reactorCount; ++reactorIndex)
		{
			const std::unique_ptr<NetworkReactor>& reactor = GetReactor(reactorIndex);
			if (reactor->GetProtocol() != serverAddress.GetProtocol())
				continue;

			return ConnectWithReactor(reactor.get());
		}

		// We don't have any reactor compatible with the server's protocol, allocate a new one
		std::size_t reactorId = AddReactor(std::make_unique<NetworkReactor>(reactorCount * MaxPeerCount, serverAddress.GetProtocol(), Nz::UInt16(0), MaxPeerCount));
		return ConnectWithReactor(GetReactor(reactorId).get());
	}

	void NetworkReactorManager::Update()
	{
		for (const auto& reactorPtr : m_reactors)
		{
			reactorPtr->Poll([&](bool outgoing, std::size_t clientId, Nz::UInt32 data) { HandlePeerConnection(outgoing, clientId, data); },
			                 [&](std::size_t clientId, Nz::UInt32 data) { HandlePeerDisconnection(clientId, data); },
			                 [&](std::size_t clientId, Nz::NetPacket&& packet) { HandlePeerPacket(clientId, packet); });
		}
	}

	void NetworkReactorManager::HandlePeerConnection(bool /*outgoing*/, std::size_t peerId, Nz::UInt32 data)
	{
		m_connections[peerId]->HandleConnection(data);
	}

	void NetworkReactorManager::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		m_connections[peerId]->HandleDisconnection(data);
		m_connections[peerId].reset();
	}

	void NetworkReactorManager::HandlePeerPacket(std::size_t peerId, Nz::NetPacket& packet)
	{
		m_connections[peerId]->HandleIncomingPacket(packet);
	}
}
