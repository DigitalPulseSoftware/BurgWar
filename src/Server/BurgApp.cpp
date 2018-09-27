// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/BurgApp.hpp>
#include <iostream>

namespace bw
{
	BurgApp::BurgApp(int argc, char* argv[]) :
	Application(argc, argv)
	{
		SetupNetwork();
	}

	int BurgApp::Run()
	{
		while (Application::Run())
		{
			m_sessionManager->Update();
		}

		return 0;
	}

	void BurgApp::SetupNetwork()
	{
		m_sessionManager.emplace(14738, 100);
	}
}
