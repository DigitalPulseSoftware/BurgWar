// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApp.hpp>
#include <Shared/NetworkSessionManager.hpp>
#include <iostream>

namespace bw
{
	ServerApp::ServerApp(int argc, char* argv[]) :
	Application(argc, argv)
	{
		m_match = std::make_unique<Match>("Je suis un match sur le serveur", 10);
		m_match->GetSessions().CreateSessionManager<NetworkSessionManager>(14768, 10);
	}

	int ServerApp::Run()
	{
		while (Application::Run())
		{
			BurgApp::Update();

			m_match->Update(GetUpdateTime());
		}

		return 0;
	}
}
