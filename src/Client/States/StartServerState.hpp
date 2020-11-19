// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_LOGINSTATE_HPP
#define BURGWAR_STATES_LOGINSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>
#include <vector>

namespace bw
{
	class StartServerState final : public AbstractState
	{
		public:
			StartServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~StartServerState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();
			void OnStartServerPressed();

			void UpdateStatus(const std::string& status, const Nz::Color& color = Nz::Color::White);

			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextState;
			Ndk::BoxLayout* m_serverConfigLayout;
			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_startServerButton;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::TextAreaWidget* m_gamemodeArea;
			Ndk::TextAreaWidget* m_mapArea;
			Ndk::TextAreaWidget* m_portArea;
	};
}

#include <Client/States/StartServerState.inl>

#endif
