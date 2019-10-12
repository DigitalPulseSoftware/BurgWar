// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientSession.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Nazara/Network/Algorithm.hpp>

namespace bw
{
	ClientSession::~ClientSession() = default;

	bool ClientSession::Connect(std::shared_ptr<SessionBridge> sessionBridge)
	{
		assert(sessionBridge);

		Disconnect();

		m_bridge = std::move(sessionBridge);

		m_onDisconnectedSlot.Connect(m_bridge->OnDisconnected, [this](Nz::UInt32 /*data*/)
		{
			OnSessionDisconnected();
		});

		m_onIncomingPacketSlot.Connect(m_bridge->OnIncomingPacket, [this](Nz::NetPacket& packet)
		{
			HandleIncomingPacket(packet);
		});

		if (!m_bridge->IsConnected())
		{
			m_onConnectedSlot.Connect(m_bridge->OnConnected, [this](Nz::UInt32 /*data*/)
			{
				OnSessionConnected();
			});
		}
		else
			OnSessionConnected();

		return true;
	}

	void ClientSession::Disconnect()
	{
		if (m_bridge)
		{
			m_bridge->Disconnect();
			OnSessionDisconnected();
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

	void ClientSession::Update()
	{
		if (m_httpDownloadManager)
			m_httpDownloadManager->Update();
	}

	void ClientSession::HandleIncomingPacket(Packets::AuthFailure&& packet)
	{
		bwLog(m_application.GetLogger(), LogLevel::Error, "Auth failed");
	}

	void ClientSession::HandleIncomingPacket(Packets::AuthSuccess&& packet)
	{
		bwLog(m_application.GetLogger(), LogLevel::Debug, "Auth succeeded");
	}

	void ClientSession::HandleIncomingPacket(Packets::ChatMessage&& packet)
	{
		if (!m_localMatch)
			return;

		m_localMatch->HandleChatMessage(std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::ClientAssetList&& packet)
	{
		if (m_state != ConnectionState::WaitingForMatchData)
			return;

		bwLog(m_application.GetLogger(), LogLevel::Info, "Received asset list");
		m_pendingPackets.assetList = std::make_unique<Packets::ClientAssetList>(std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::ClientScriptList&& packet)
	{
		if (m_state != ConnectionState::WaitingForMatchData)
			return;

		bwLog(m_application.GetLogger(), LogLevel::Info, "Received client script list");
		m_pendingPackets.scriptList = std::make_unique<Packets::ClientScriptList>(std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::ConsoleAnswer&& packet)
	{
		m_localMatch->HandleConsoleAnswer(std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::ControlEntity&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::CreateEntities&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::DeleteEntities&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::DownloadClientScriptResponse&& packet)
	{
		assert(m_downloadManager.has_value());
		m_downloadManager->HandlePacket(packet);
	}

	void ClientSession::HandleIncomingPacket(Packets::EntitiesInputs&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::EntityWeapon&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::HealthUpdate&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::HelloWorld&& packet)
	{
		bwLog(m_application.GetLogger(), LogLevel::Info, "Hello world: {0}", packet.str);
	}

	void ClientSession::HandleIncomingPacket(Packets::InputTimingCorrection&& packet)
	{
		//std::cout << "[Client] Server tick error: " << packet.tickError << std::endl;
		m_localMatch->HandleTickError(packet.serverTick, packet.tickError);
	}

	void ClientSession::HandleIncomingPacket(Packets::MatchData&& matchData)
	{
		if (m_state != ConnectionState::WaitingForMatchData)
			return;

		// pls lynix-san refactor with state machine

		bwLog(m_application.GetLogger(), LogLevel::Info, "Received math data");
		m_pendingPackets.matchData = std::make_unique<Packets::MatchData>(std::move(matchData));

		// Switch to downloading assets
		if (!m_pendingPackets.assetList)
		{
			bwLog(m_application.GetLogger(), LogLevel::Error, "Expected asset list packet before match data, failed to load match");
			Disconnect();
			return;
		}

		m_state = ConnectionState::DownloadingAssets;
		bwLog(m_application.GetLogger(), LogLevel::Info, "Downloading assets...");

		auto resourceDirectory = std::make_shared<VirtualDirectory>(m_application.GetConfig().GetStringOption("Assets.ResourceFolder"));
		auto targetResourceDirectory = std::make_shared<VirtualDirectory>();

		m_httpDownloadManager.emplace(m_localMatch->GetLogger(), ".assetCache", std::move(m_pendingPackets.assetList->fastDownloadUrls), resourceDirectory);

		m_httpDownloadManager->OnFileChecked.Connect([this, targetResourceDirectory](HttpDownloadManager* /*downloadManager*/, const std::string& resourcePath, const std::filesystem::path& realPath)
		{
			targetResourceDirectory->StoreFile(resourcePath, realPath);
		});

		m_httpDownloadManager->OnFileCheckedMemory.Connect([this, targetResourceDirectory](HttpDownloadManager* /*downloadManager*/, const std::string& resourcePath, const std::vector<Nz::UInt8>& content)
		{
			targetResourceDirectory->StoreFile(resourcePath, content);
		});

		m_httpDownloadManager->OnFinished.Connect([this, targetResourceDirectory](HttpDownloadManager* downloadManager) mutable
		{
			m_state = ConnectionState::DownloadingScripts;
			bwLog(m_application.GetLogger(), LogLevel::Info, "Downloading scripts...");

			m_downloadManager.emplace(".scriptCache");

			m_scriptDirectory = std::make_shared<VirtualDirectory>();

			m_downloadManager->OnFileChecked.Connect([this](ClientScriptDownloadManager* downloadManager, const std::string& filePath, const std::vector<Nz::UInt8>& fileContent)
			{
				m_scriptDirectory->StoreFile(filePath, fileContent);
			});

			m_downloadManager->OnDownloadRequest.Connect([this](ClientScriptDownloadManager* downloadManager, const Packets::DownloadClientScriptRequest& request)
			{
				SendPacket(request);
			});

			m_downloadManager->OnFinished.Connect([this, targetResourceDirectory](ClientScriptDownloadManager* downloadManager) mutable
			{
				m_state = ConnectionState::Ready;
				bwLog(m_application.GetLogger(), LogLevel::Info, "Creating match...");

				m_localMatch = m_matchFactory(*this, std::move(*m_pendingPackets.matchData));
				m_localMatch->LoadAssets(std::move(targetResourceDirectory));
				m_localMatch->LoadScripts(m_scriptDirectory);
				
				m_downloadManager.reset();
			});

			m_downloadManager->HandlePacket(*m_pendingPackets.scriptList);
			m_pendingPackets.scriptList.reset();

			m_httpDownloadManager.reset();
		});

		for (const auto& asset : m_pendingPackets.assetList->assets)
			m_httpDownloadManager->RegisterFile(asset.path, asset.sha1Checksum, asset.size);
		
		m_httpDownloadManager->Start();

		m_pendingPackets.assetList.reset();
	}

	void ClientSession::HandleIncomingPacket(Packets::MatchState&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::NetworkStrings&& packet)
	{
		if (packet.startId == 0)
			m_stringStore.Clear(); //< Reset string store

		m_stringStore.FillStore(packet.startId, packet.strings);
	}

	void ClientSession::HandleIncomingPacket(Packets::PlayerWeapons&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::HandleIncomingPacket(Packets::EntitiesAnimation&& packet)
	{
		Nz::UInt16 stateTick = packet.stateTick;
		m_localMatch->PushTickPacket(stateTick, std::move(packet));
	}

	void ClientSession::OnSessionConnected()
	{
		bwLog(m_application.GetLogger(), LogLevel::Info, "Connected");

		OnConnected(this);

		Packets::Auth auth;
		auto& playerData = auth.players.emplace_back();
		playerData.nickname = m_playerName;

		SendPacket(auth);
	}

	void ClientSession::OnSessionDisconnected()
	{
		m_bridge.reset();
		OnDisconnected(this);
	}
}
