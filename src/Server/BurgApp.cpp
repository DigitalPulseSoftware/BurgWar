// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/BurgApp.hpp>
#include <Shared/NetworkSessionManager.hpp>
#include <Shared/Components/HealthComponent.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <iostream>

namespace bw
{
	BurgApp::BurgApp(int argc, char* argv[]) :
	Application(argc, argv)
	{
		Ndk::InitializeComponent<HealthComponent>("Health");
		Ndk::InitializeComponent<NetworkSyncComponent>("NetSync");
		Ndk::InitializeComponent<PlayerControlledComponent>("PlyCtrl");
		Ndk::InitializeComponent<PlayerMovementComponent>("PlyMvt");
		Ndk::InitializeComponent<ScriptComponent>("Script");
		Ndk::InitializeSystem<NetworkSyncSystem>();
		Ndk::InitializeSystem<PlayerControlledSystem>();
		Ndk::InitializeSystem<PlayerMovementSystem>();

		m_match = std::make_unique<Match>("Je suis un match sur le serveur", 10);
		m_match->GetSessions().CreateSessionManager<NetworkSessionManager>(14768, 10);
	}

	int BurgApp::Run()
	{
		while (Application::Run())
		{
			m_match->Update(GetUpdateTime());
		}

		return 0;
	}
}
