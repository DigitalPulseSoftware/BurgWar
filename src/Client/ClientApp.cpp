// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApp.hpp>
#include <CoreLib/Match.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/KeyboardAndMouseController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <iostream>

namespace bw
{
	ClientApp::ClientApp(int argc, char* argv[]) :
	Application(argc, argv),
	m_mainWindow(AddWindow<Nz::RenderWindow>(Nz::VideoMode(1280, 720), "Burg'war", Nz::WindowStyle_Default, Nz::RenderTargetParameters(8)))
	{
		m_mainWindow.EnableVerticalSync(false);
		m_mainWindow.SetFramerateLimit(100);

		m_match = std::make_unique<Match>(*this, "Faites l'amour pas la Burg'guerre", "test", 10);
		LocalSessionManager* localSessions = m_match->GetSessions().CreateSessionManager<LocalSessionManager>();

		auto CreateMatch = [this](ClientSession& session, const Packets::MatchData& matchData) -> std::shared_ptr<LocalMatch>
		{
			auto inputController = std::make_shared<KeyboardAndMouseController>(m_mainWindow);

			return m_localMatches.emplace_back(std::make_shared<LocalMatch>(*this, &m_mainWindow, session, matchData, inputController));
		};

		m_clientSession = std::make_unique<ClientSession>(*this, m_commandStore, CreateMatch);
		m_clientSession->Connect(localSessions->CreateSession());
	}

	ClientApp::~ClientApp() = default;

	int ClientApp::Run()
	{
		while (Application::Run())
		{
			m_mainWindow.Display();

			BurgApp::Update();

			m_networkReactors.Update();

			if (m_match)
				m_match->Update(GetUpdateTime());

			for (const auto& localMatchPtr : m_localMatches)
				localMatchPtr->Update(GetUpdateTime());
		}

		return 0;
	}
}
