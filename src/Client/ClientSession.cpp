// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientSession.hpp>
#include <Shared/NetworkSessionBridge.hpp>
#include <Shared/Utility/VirtualDirectory.hpp>
#include <Client/ClientApp.hpp>
#include <Client/LocalCommandStore.hpp>
#include <Client/LocalCommandStore.hpp>
#include <Client/LocalMatch.hpp>
#include <Client/LocalSessionBridge.hpp>
#include <Client/LocalSessionManager.hpp>
#include <Nazara/Network/Algorithm.hpp>

namespace bw
{
	ClientSession::~ClientSession() = default;

	bool ClientSession::Connect(LocalSessionManager* sessionManager)
	{
		Disconnect();

		auto bridge = sessionManager->CreateSession();
		if (!bridge)
			return false;

		ConnectInternal(std::move(bridge));

		OnConnected();
		return true;
	}

	bool ClientSession::Connect(const Nz::IpAddress& address)
	{
		Disconnect();

		auto bridge = m_application.ConnectNewServer(address, 0);
		if (!bridge)
			return false;

		m_onConnectedSlot.Connect(bridge->OnConnected, [this](Nz::UInt32 /*data*/) 
		{
			OnConnected();
		});

		ConnectInternal(std::move(bridge));
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
			OnDisconnected();
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

	void ClientSession::ConnectInternal(std::shared_ptr<SessionBridge> sessionBridge)
	{
		m_bridge = std::move(sessionBridge);
		m_onDisconnectedSlot.Connect(m_bridge->OnDisconnected, [this](Nz::UInt32 /*data*/)
		{
			OnDisconnected();
		});

		m_onIncomingPacketSlot.Connect(m_bridge->OnIncomingPacket, [this](Nz::NetPacket& packet)
		{
			HandleIncomingPacket(packet);
		});
	}

	void ClientSession::HandleIncomingPacket(const Packets::AuthFailure& packet)
	{
		std::cout << "[Client] Auth failed" << std::endl;
	}

	void ClientSession::HandleIncomingPacket(const Packets::AuthSuccess& packet)
	{
		std::cout << "[Client] Auth succeeded" << std::endl;
	}

	void ClientSession::HandleIncomingPacket(const Packets::ClientScriptList& packet)
	{
		std::cout << "[Client] Got client script list" << std::endl;

		assert(!m_downloadManager.has_value());
		m_downloadManager.emplace(".scriptCache");

		m_scriptDirectory = std::make_shared<VirtualDirectory>();

		m_downloadManager->OnFileChecked.Connect([this](ClientScriptDownloadManager* downloadManager, const std::string& filePath, const std::vector<Nz::UInt8>& fileContent)
		{
			m_scriptDirectory->Store(filePath, fileContent);
		});

		m_downloadManager->OnDownloadRequest.Connect([this](ClientScriptDownloadManager* downloadManager, const Packets::DownloadClientScriptRequest& request)
		{
			SendPacket(request);
		});

		m_downloadManager->OnFinished.Connect([this](ClientScriptDownloadManager* downloadManager)
		{
			m_localMatch->LoadScripts(m_scriptDirectory);
			SendPacket(Packets::Ready{});
		});

		m_downloadManager->HandlePacket(packet);
	}

	void ClientSession::HandleIncomingPacket(const Packets::ControlEntity& packet)
	{
		std::cout << "[Client] Control entity #" << packet.entityId << std::endl;
		m_localMatch->ControlEntity(packet.entityId);
	}

	void ClientSession::HandleIncomingPacket(const Packets::CreateEntities& packet)
	{
		for (const auto& entityData : packet.entities)
		{
			const std::string& entityClass = m_stringStore.GetString(entityData.entityClass);
			std::cout << "[Client] Entity #" << entityData.id << " of type " << entityClass << " created at " << entityData.position << std::endl;

			Nz::UInt16 currentHealth = 0;
			Nz::UInt16 maxHealth = 0;

			if (entityData.health.has_value())
			{
				currentHealth = entityData.health->currentHealth;
				maxHealth = entityData.health->maxHealth;
			}

			m_localMatch->CreateEntity(entityData.id, entityClass, entityData.position, entityData.playerMovement.has_value(), entityData.inputs.has_value(), entityData.physicsProperties.has_value(), entityData.parentId, currentHealth, maxHealth);
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

	void ClientSession::HandleIncomingPacket(const Packets::DownloadClientScriptResponse& packet)
	{
		assert(m_downloadManager.has_value());
		m_downloadManager->HandlePacket(packet);
	}

	void ClientSession::HandleIncomingPacket(const Packets::EntitiesInputs& packet)
	{
		for (const auto& entityData : packet.entities)
		{
			//std::cout << "[Client] Input update for entity " << entityData.id << std::endl;
			m_localMatch->UpdateEntityInput(entityData.id, entityData.inputs);
		}
	}

	void ClientSession::HandleIncomingPacket(const Packets::HealthUpdate& packet)
	{
		for (const auto& entityData : packet.entities)
		{
			std::cout << "[Client] Health update for entity " << entityData.id << " (now at " << entityData.currentHealth << ')' << std::endl;
			m_localMatch->UpdateEntityHealth(entityData.id, entityData.currentHealth);
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
			bool isFacingRight = false;

			//std::cout << "[Client] Entity #" << entityData.id << " is now at " << entityData.position;
			if (entityData.playerMovement.has_value())
			{
				isFacingRight = entityData.playerMovement->isFacingRight;
				//std::cout << " and has player entity data (facing right=" << isFacingRight << ")";
			}

			//std::cout << "\n";

			Nz::RadianAnglef angularVelocity = 0.f;
			Nz::Vector2f linearVelocity = Nz::Vector2f::Zero();

			if (entityData.physicsProperties.has_value())
			{
				angularVelocity = entityData.physicsProperties->angularVelocity;
				linearVelocity = entityData.physicsProperties->linearVelocity;
			}

			m_localMatch->MoveEntity(entityData.id, entityData.position, linearVelocity, entityData.rotation, angularVelocity, isFacingRight);
		}
	}

	void ClientSession::HandleIncomingPacket(const Packets::NetworkStrings& packet)
	{
		if (packet.startId == 0)
			m_stringStore.Clear(); //< Reset string store

		m_stringStore.FillStore(packet.startId, packet.strings);
	}

	void ClientSession::HandleIncomingPacket(const Packets::PlayAnimation& packet)
	{
		std::cout << "[Client] Entity #" << packet.entityId << " plays animation " << +packet.animId << std::endl;
		m_localMatch->PlayAnimation(packet.entityId, packet.animId);
	}

	void ClientSession::OnConnected()
	{
		std::cout << "Connected" << std::endl;

		m_isConnected = true;

		Packets::Auth auth;
		auth.playerCount = 1;

		SendPacket(auth);
	}

	void ClientSession::OnDisconnected()
	{
		m_bridge.reset();
		m_isConnected = false;
	}
}
