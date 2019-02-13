// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTAPP_HPP
#define BURGWAR_CLIENTAPP_HPP

#include <CoreLib/BurgApp.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/NetworkReactorManager.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Application.hpp>
#include <vector>

namespace bw
{
	class LocalMatch;
	class Match;
	class NetworkReactor;
	class NetworkSessionBridge;

	class ClientApp : public Ndk::Application, public BurgApp
	{
		public:
			ClientApp(int argc, char* argv[]);
			~ClientApp();

			int Run();

		private:
			std::vector<std::shared_ptr<LocalMatch>> m_localMatches;
			std::unique_ptr<Match> m_match;
			std::unique_ptr<ClientSession> m_clientSession;
			Nz::RenderWindow& m_mainWindow;
			LocalCommandStore m_commandStore;
			NetworkReactorManager m_networkReactors;
	};
}

#include <Client/ClientApp.inl>

#endif