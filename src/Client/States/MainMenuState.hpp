// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_MAINMENUSTATE_HPP
#define BURGWAR_STATES_MAINMENUSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <vector>

namespace bw
{
	class MainMenuState final : public AbstractState
	{
		public:
			MainMenuState(std::shared_ptr<StateData> stateData);
			~MainMenuState() = default;

		private:
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnJoinServerPressed();
			void OnOptionPressed();
			void OnStartServerPressed();
			void OnQuitPressed();

			std::shared_ptr<AbstractState> m_nextState;
			Ndk::BoxLayout* m_serverAddressLayout;
			Nz::ButtonWidget* m_joinServerButton;
			Nz::ButtonWidget* m_optionButton;
			Nz::ButtonWidget* m_quitButton;
			Nz::ButtonWidget* m_startServerButton;
	};
}

#include <Client/States/MainMenuState.inl>

#endif
