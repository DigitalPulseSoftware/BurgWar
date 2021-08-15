// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/ServerListState.hpp>
#include <CoreLib/WebRequest.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/JoinServerState.hpp>
#include <NDK/StateMachine.hpp>
#include <nlohmann/json.hpp>

namespace bw
{
	constexpr float RefreshTime = 15.f;

	ServerListState::ServerListState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
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

		m_serverListData = std::make_shared<ServerData>();

		const std::string& masterServerList = GetStateData().app->GetConfig().GetStringValue("GameSettings.MasterServers");
		SplitStringAny(masterServerList, "\f\n\r\t\v ", [&](const std::string_view& masterServerURI)
		{
			if (!masterServerURI.empty())
				m_serverListData->masterServers.emplace(masterServerURI, 0.f);

			return true;
		});
	}

	bool ServerListState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		/*
		if (m_nextGameState)
			fsm.ResetState(std::move(m_nextGameState));
		*/

		if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		RefreshServers(elapsedTime);

		return true;
	}

	void ServerListState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 2> widgets = {
			m_backButton,
			m_directConnectButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
			totalSize += widget->GetSize().x;

		Nz::Vector2f cursor = center;
		cursor.x -= totalSize / 2.f;

		m_backButton->SetPosition({ cursor.x, 0.f, 0.f });
		m_backButton->CenterVertical();
		cursor.x += m_backButton->GetSize().x + padding;

		m_directConnectButton->SetPosition({ cursor.x, 0.f, 0.f });
		m_directConnectButton->CenterVertical();
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

	void ServerListState::RefreshServers(float elapsedTime)
	{
		for (auto it = m_serverListData->masterServers.begin(); it != m_serverListData->masterServers.end(); ++it)
		{
			const std::string& masterServer = it->first;
			float& timeBeforeRefresh = it.value();

			timeBeforeRefresh -= elapsedTime;
			if (timeBeforeRefresh < 0.f)
			{
				timeBeforeRefresh = RefreshTime / 2.f;

				std::unique_ptr<WebRequest> request = WebRequest::Get(masterServer + "/servers", [stateData = GetStateDataPtr(), url = masterServer, serverDataPtr = std::weak_ptr(m_serverListData)](WebRequestResult&& result)
				{
					if (!result)
					{
						bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to refresh server list from {0}, register request failed: {1}", url, result.GetErrorMessage());
						return;
					}

					switch (result.GetReponseCode())
					{
						case 200:
							bwLog(stateData->app->GetLogger(), LogLevel::Debug, "successfully refreshed server list from {0}", url);
							
							if (auto serverData = serverDataPtr.lock())
							{
								nlohmann::json serverList = nlohmann::json::parse(result.GetBody());
								for (auto&& serverDoc : serverList)
								{
									bwLog(stateData->app->GetLogger(), LogLevel::Debug, " - {0}: {1}:{2} ", serverDoc.value("name", "<noname>"), serverDoc.value("address", "<noip>"), serverDoc.value("port", 0));
								}

								auto it = serverData->masterServers.find(url);
								if (it != serverData->masterServers.end())
									it.value() = RefreshTime;
							}
							break;

						default:
							bwLog(stateData->app->GetLogger(), LogLevel::Error, "failed to refresh server list from {0}, request failed with code {1}", url, result.GetReponseCode());
							break;
					}
				});

				GetStateData().app->GetWebService().AddRequest(std::move(request));
			}
		}
	}
}
