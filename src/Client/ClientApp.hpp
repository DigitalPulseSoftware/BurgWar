// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTAPP_HPP
#define BURGWAR_CLIENTAPP_HPP

#include <ClientLib/ClientEditorAppComponent.hpp>
#include <ClientLib/NetworkReactorManager.hpp>
#include <Client/ClientAppConfig.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Application.hpp>
#include <NDK/StateMachine.hpp>
#include <vector>

namespace bw
{
	struct StateData;

	class ClientApp : public ClientEditorAppComponent
	{
		public:
			ClientApp(int argc, char* argv[]);
			~ClientApp();

			inline NetworkReactorManager& GetReactorManager();

			int Run();

		private:
			std::shared_ptr<StateData> m_stateData;
			Ndk::StateMachine m_stateMachine;
			Nz::RenderWindow* m_mainWindow;
			ClientAppConfig m_configFile;
			NetworkReactorManager m_networkReactors;
	};
}

#include <Client/ClientApp.inl>

#endif
