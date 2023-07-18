// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTAPP_HPP
#define BURGWAR_CLIENTAPP_HPP

#include <ClientLib/ClientEditorAppComponent.hpp>
#include <ClientLib/NetworkReactorManager.hpp>
#include <Client/ClientAppConfig.hpp>
#include <Nazara/Core/ApplicationBase.hpp>
#include <Nazara/Renderer/WindowSwapchain.hpp>
#include <Nazara/Platform/Window.hpp>
#include <Nazara/Core/StateMachine.hpp>
#include <optional>
#include <vector>

namespace bw
{
	struct StateData;

	class ClientAppComponent : public ClientEditorAppComponent
	{
		public:
			ClientAppComponent(Nz::ApplicationBase& app, int argc, char* argv[]);
			~ClientAppComponent();

			inline NetworkReactorManager& GetReactorManager();

			void Update(Nz::Time elapsedTime) override;

		private:
			std::shared_ptr<StateData> m_stateData;
			Nz::StateMachine m_stateMachine;
			Nz::Window* m_mainWindow;
			ClientAppConfig m_configFile;
			NetworkReactorManager m_networkReactors;
	};
}

#include <Client/ClientAppComponent.inl>

#endif
