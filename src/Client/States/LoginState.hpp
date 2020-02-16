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
	class LoginState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~LoginState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;

			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnConnectionPressed();
			void OnStartServerPressed();
			void OnQuitPressed();

			void UpdateStatus(const std::string& status, const Nz::Color& color = Nz::Color::White);

			std::shared_ptr<AbstractState> m_nextState;
			Ndk::BoxLayout* m_serverAddressLayout;
			Ndk::ButtonWidget* m_connectionButton;
			Ndk::ButtonWidget* m_startServerButton;
			Ndk::ButtonWidget* m_quitButton;
			Ndk::LabelWidget* m_loginLabel;
			Ndk::LabelWidget* m_serverLabel;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::TextAreaWidget* m_loginArea;
			Ndk::TextAreaWidget* m_serverAddressArea;
			Ndk::TextAreaWidget* m_serverPortArea;
	};
}

#include <Client/States/LoginState.inl>

#endif
