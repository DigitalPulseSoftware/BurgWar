// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/ServerListState.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/WebRequest.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/JoinServerState.hpp>
#include <Client/States/Game/ConnectionState.hpp>
#include <Nazara/Utility/RichTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <nlohmann/json.hpp>

namespace bw
{
	namespace
	{
		constexpr Nz::UInt32 MasterServerDataVersion = 1U;
	}

	constexpr float RefreshTime = 15.f;

	ServerListState::ServerListState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
		if (WebService::IsInitialized())
			m_webService.emplace(GetStateData().app->GetLogger());
		else
			bwLog(GetStateData().app->GetLogger(), LogLevel::Warning, "web services are not initialized, server listing will not work");

		m_serverListWidget = GetStateData().canvas->Add<Nz::BaseWidget>();

		m_serverListScrollbar = CreateWidget<Ndk::ScrollAreaWidget>(m_serverListWidget);
		m_serverListScrollbar->EnableBackground(true);
		m_serverListScrollbar->SetBackgroundColor(Nz::Color(0, 0, 0, 80));
		m_serverListScrollbar->EnableScrollbar(true);

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->Resize(m_backButton->GetPreferredSize());
		
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		m_directConnectButton = CreateWidget<Ndk::ButtonWidget>();
		m_directConnectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Direct connection...", 24));
		m_directConnectButton->Resize(m_directConnectButton->GetPreferredSize());
		
		m_directConnectButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnDirectConnectionPressed();
		});

		const std::string& masterServerList = GetStateData().app->GetConfig().GetStringValue("ServerSettings.MasterServers");
		SplitStringAny(masterServerList, "\f\n\r\t\v ", [&](const std::string_view& masterServerURI)
		{
			if (!masterServerURI.empty())
				m_masterServers.emplace(masterServerURI, MasterServerData{});

			return true;
		});
	}

	ServerListState::~ServerListState()
	{
		//FIXME: ScrollAreaWidget uses SetParent which is broken as it doesn't register children to parent
		m_serverListWidget->SetParent(&GetStateData().canvas.value());
		m_serverListWidget->Destroy();
	}

	void ServerListState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		m_serverListWidget->Show();
	}

	void ServerListState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_serverListWidget->Hide();
	}

	bool ServerListState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextGameState)
			fsm.ResetState(std::move(m_nextGameState));

		if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		if (m_webService)
		{
			m_webService->Poll();
			RefreshServers(elapsedTime);
		}

		return true;
	}

	void ServerListState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		m_serverListScrollbar->Resize(canvasSize * 0.5f);
		m_serverListScrollbar->Center();

		constexpr float padding = 10.f;

		m_tempOrderedServerList.clear();

		for (auto&& [url, masterServerData] : m_masterServers)
		{
			for (auto&& [uuid, serverData] : masterServerData.serverList)
				m_tempOrderedServerList.emplace_back(serverData);
		}

		std::sort(m_tempOrderedServerList.begin(), m_tempOrderedServerList.end(), [](const ServerData& lhs, const ServerData& rhs)
		{
			return lhs.serverName < rhs.serverName;
		});

		float maxWidth = 0.f;
		Nz::Vector2f heightCursor = { 0.f, 0.f };
		for (const ServerData& serverData : m_tempOrderedServerList)
		{
			Nz::Vector2f maxSize = { 0.f, 0.f };

			maxSize.Maximize(serverData.infoLabel->GetSize());
			serverData.infoLabel->SetPosition(heightCursor);

			maxSize.y = std::max(maxSize.y, serverData.connectButton->GetSize().y);
			serverData.connectButton->SetPosition(heightCursor + Nz::Vector2f(maxSize.x + padding, maxSize.y / 2.f - serverData.connectButton->GetSize().y / 2.f));
			maxSize.x += padding + serverData.connectButton->GetSize().x;

			maxWidth = std::max(maxWidth, maxSize.x);
			heightCursor.y += maxSize.y + padding;
		}

		m_serverListWidget->Resize({ maxWidth, heightCursor.y });

		std::array<Nz::BaseWidget*, 2> widgets = {
			m_backButton,
			m_directConnectButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Nz::BaseWidget* widget : widgets)
			totalSize += widget->GetSize().x;

		Nz::Vector2f cursor;
		cursor.x = center.x - totalSize / 2.f;
		cursor.y = m_serverListScrollbar->GetPosition(Nz::CoordSys::Global).y + m_serverListScrollbar->GetSize().y + padding;

		m_backButton->SetPosition({ cursor.x, cursor.y, 0.f });
		cursor.x += m_backButton->GetSize().x + padding;

		m_directConnectButton->SetPosition({ cursor.x, cursor.y, 0.f });
		cursor.x += m_directConnectButton->GetSize().x + padding;
	}

	void ServerListState::OnBackPressed()
	{
		m_nextState = std::move(m_previousState);
	}

	void ServerListState::OnDirectConnectionPressed()
	{
		m_nextState = std::make_shared<JoinServerState>(GetStateDataPtr(), shared_from_this());
	}

	void ServerListState::OnServerConnectionPressed(const std::string& masterServer, const std::string& uuid)
	{
		std::unique_ptr<WebRequest> request = WebRequest::Get(masterServer + "/server/" + uuid + "/connection_details", [this, stateData = GetStateDataPtr(), masterServer, uuid](WebRequestResult&& result)
		{
			if (!result)
			{
				bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to connect to server {0} from {1}, connect request failed: {2}", uuid, masterServer, result.GetErrorMessage());
				return;
			}

			switch (result.GetReponseCode())
			{
				case 200:
				{
					bwLog(stateData->app->GetLogger(), LogLevel::Debug, "successfully received connection info of {0} from {1}", uuid, masterServer);
					nlohmann::json connectionDetails;
					
					try
					{
						connectionDetails = nlohmann::json::parse(result.GetBody());
					}
					catch (const std::exception& e)
					{
						bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to parse connection details: {0}", e.what());
						return;
					}

					Nz::UInt32 dataVersion = connectionDetails.value("data_version", 0);
					if (dataVersion != MasterServerDataVersion)
						bwLog(stateData->app->GetLogger(), LogLevel::Warning, "unexpected data version (expected {0}, got {1})", MasterServerDataVersion, dataVersion);

					std::string addresses = connectionDetails.value("ip", "");
					if (addresses.empty())
					{
						bwLog(stateData->app->GetLogger(), LogLevel::Error, "missing ip field, aborting connection.");
						return;
					}

					Nz::UInt16 serverPort = connectionDetails.value("port", Nz::UInt16(0));
					if (serverPort == 0)
					{
						bwLog(stateData->app->GetLogger(), LogLevel::Error, "missing or invalid port field ({0}), aborting connection.", serverPort);
						return;
					}

					ConnectionState::AddressList serverAddresses;
					SplitString(addresses, ";", [&](const std::string_view& ip)
					{
						ConnectionState::ServerName address;
						address.hostname = ip;
						address.port = serverPort;

						serverAddresses.emplace_back(std::move(address));
						return true;
					});

					m_nextGameState = std::make_shared<ConnectionState>(GetStateDataPtr(), std::move(serverAddresses), shared_from_this());
					break;
				}

				default:
					bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to connect to server {0} from {1}, request failed with code {2}", uuid, masterServer, result.GetReponseCode());
					break;
			}
		});

		m_webService->AddRequest(std::move(request));
	}

	void ServerListState::RefreshServers(float elapsedTime)
	{
		for (auto it = m_masterServers.begin(); it != m_masterServers.end(); ++it)
		{
			const std::string& masterServer = it->first;
			MasterServerData& masterServerData = it.value();

			masterServerData.timeBeforeRefresh -= elapsedTime;
			if (masterServerData.timeBeforeRefresh < 0.f)
			{
				masterServerData.timeBeforeRefresh = RefreshTime / 2.f;

				std::unique_ptr<WebRequest> request = WebRequest::Get(masterServer + "/servers", [this, stateData = GetStateDataPtr(), url = masterServer](WebRequestResult&& result)
				{
					if (!result)
					{
						bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to refresh server list from {0}, register request failed: {1}", url, result.GetErrorMessage());
						return;
					}

					switch (result.GetReponseCode())
					{
						case 200:
						{
							bwLog(stateData->app->GetLogger(), LogLevel::Debug, "successfully refreshed server list from {0}", url);
							
							UpdateServerList(url, nlohmann::json::parse(result.GetBody()));

							auto it = m_masterServers.find(url);
							if (it != m_masterServers.end())
								it.value().timeBeforeRefresh = RefreshTime;

							break;
						}

						default:
							bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to refresh server list from {0}, request failed with code {1}", url, result.GetReponseCode());
							break;
					}
				});

				m_webService->AddRequest(std::move(request));
			}
		}
	}

	void ServerListState::UpdateServerList(const std::string& masterServer, const nlohmann::json& serverListDoc)
	{
		StateData& stateData = GetStateData();

		auto it = m_masterServers.find(masterServer);
		if (it == m_masterServers.end())
			return;

		MasterServerData& masterServerData = it.value();

		Nz::UInt32 dataVersion = serverListDoc.value("data_version", 0);
		if (dataVersion != MasterServerDataVersion)
			bwLog(stateData.app->GetLogger(), LogLevel::Warning, "unexpected data version (expected {0}, got {1})", MasterServerDataVersion, dataVersion);

		tsl::hopscotch_map<std::string, ServerData> newServerList;

		for (auto&& serverDoc : serverListDoc["servers"])
		{
			std::string uuid = serverDoc.value("uuid", "");
			if (uuid.empty())
			{
				bwLog(stateData.app->GetLogger(), LogLevel::Error, "got server without mandatory uuid field");
				continue;
			}

			Nz::UInt32 version = serverDoc.value("game_version", Nz::UInt32(0));
			if (version != GameVersion)
			{
				bwLog(stateData.app->GetLogger(), LogLevel::Debug, "ignored server {0} because version didn't match (got {1}, expected {2})", uuid, version, GameVersion);
				continue;
			}

			std::string name = serverDoc.value("name", "<no name>");
			std::string desc = serverDoc.value("description", "");
			std::string gamemode = serverDoc.value("gamemode", "<no gamemode>");
			std::string map = serverDoc.value("map", "<no map>");
			Nz::UInt32 playerCount = serverDoc.value("current_player_count", Nz::UInt32(0));
			Nz::UInt32 maxPlayerCount = serverDoc.value("maximum_player_count", Nz::UInt32(0));

			ServerData serverData;

			auto serverIt = masterServerData.serverList.find(uuid);
			if (serverIt != masterServerData.serverList.end())
			{
				// Update server
				serverData = std::move(serverIt.value());
				masterServerData.serverList.erase(serverIt);
			}
			else
			{
				if (masterServerData.receivedData)
					bwLog(stateData.app->GetLogger(), LogLevel::Debug, "server {0} appeared (from {1})", uuid, masterServer);

				// New server
				serverData.connectButton = m_serverListWidget->Add<Ndk::ButtonWidget>();
				serverData.connectButton->UpdateText(Nz::SimpleTextDrawer::Draw("Connect", 24));
				serverData.connectButton->Resize(serverData.connectButton->GetPreferredSize());
				serverData.connectButton->OnButtonTrigger.Connect([this, masterServer, uuid](const Ndk::ButtonWidget* /*button*/)
				{
					OnServerConnectionPressed(masterServer, uuid);
				});

				serverData.infoLabel = m_serverListWidget->Add<Ndk::LabelWidget>();
			}

			Nz::RichTextDrawer infoDrawer;
			infoDrawer.SetDefaultCharacterSize(24);
			infoDrawer.AppendText(name + '\n');

			infoDrawer.SetDefaultCharacterSize(18);
			infoDrawer.SetDefaultColor(Nz::Color::White);
			if (!desc.empty())
				infoDrawer.AppendText(desc + '\n');

			infoDrawer.SetDefaultCharacterSize(18);

			infoDrawer.SetDefaultColor(Nz::Color(220, 220, 220));
			infoDrawer.AppendText("Gamemode: ");

			infoDrawer.SetDefaultColor(Nz::Color::White);
			infoDrawer.AppendText(gamemode);

			infoDrawer.SetDefaultColor(Nz::Color(220, 220, 220));
			infoDrawer.AppendText(" Map: ");

			infoDrawer.SetDefaultColor(Nz::Color::White);
			infoDrawer.AppendText(map);

			infoDrawer.AppendText("\n");

			infoDrawer.SetDefaultCharacterSize(16);
			infoDrawer.AppendText(std::to_string(playerCount) + "/" + std::to_string(maxPlayerCount) + " players");

			serverData.infoLabel->UpdateText(infoDrawer);
			serverData.infoLabel->Resize(serverData.infoLabel->GetPreferredSize());

			serverData.serverName = std::move(name);

			newServerList.emplace(std::move(uuid), std::move(serverData));
		}

		for (auto&& [uuid, serverData] : masterServerData.serverList)
		{
			bwLog(stateData.app->GetLogger(), LogLevel::Debug, "server {0} disappeared (from {1})", uuid, masterServer);

			// Delete old servers
			serverData.connectButton->Destroy();
			serverData.infoLabel->Destroy();
		}

		if (!masterServerData.receivedData)
		{
			bwLog(stateData.app->GetLogger(), LogLevel::Debug, "received {0} server(s) from {1}", newServerList.size(), masterServer);
			masterServerData.receivedData = true;
		}

		masterServerData.serverList = std::move(newServerList);

		LayoutWidgets();
	}
}
