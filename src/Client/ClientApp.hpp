// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTAPP_HPP
#define BURGWAR_CLIENTAPP_HPP

#include <ClientLib/ClientEditorApp.hpp>
#include <ClientLib/NetworkReactorManager.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Application.hpp>
#include <NDK/StateMachine.hpp>
#include <vector>

namespace bw
{
	struct StateData;

	class ClientApp : public ClientEditorApp
	{
		public:
			ClientApp(int argc, char* argv[]);
			~ClientApp();

			inline NetworkReactorManager& GetReactorManager();

			int Run();

		private:
			void RegisterClientConfig();

			std::shared_ptr<StateData> m_stateData;
			Ndk::StateMachine m_stateMachine;
			Nz::RenderWindow* m_mainWindow;
			NetworkReactorManager m_networkReactors;
	};
}

#include <Client/ClientApp.inl>

#endif
